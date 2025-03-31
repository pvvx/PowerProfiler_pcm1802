#include "main_config.h"
#if USE_I2S
#include "board.h"
#include "bflb_clock.h"
#include "bflb_mtimer.h"
#include "bflb_gpio.h"
#include "bflb_i2s.h"
#include "bflb_dma.h"
#include "bl702_glb.h"
#include "usb_buffer.h"
#include "i2s_drv.h"
#include "hardware/dma_reg.h"
#include "..\SDK\drivers\soc\bl702\std\include\hardware\glb_reg.h"
 #include "bflb_clock.h"
#include "hardware/i2s_reg.h"

#if USE_BOARD == bl702rd
#define GPIO_I2S_POW  GPIO_PIN_26	// (RX) POW Power-Down Control PCM1802
#define GPIO_I2S_BCLK GPIO_PIN_24	// (RTS) BCK 3.0724 MHz
#define GPIO_I2S_RCLK GPIO_PIN_27	// (TX)  SCK 24.5792 MHz
#define GPIO_I2S_DI   GPIO_PIN_2	// (TMS) DIO/DO
#define GPIO_I2S_FS   GPIO_PIN_1	// (TDO) LRCK 48.0062 kHz
#elif USE_BOARD == bl702zb
#define GPIO_I2S_POW  GPIO_PIN_9	// POW Power-Down Control PCM1802
#define GPIO_I2S_BCLK GPIO_PIN_0	// BCLK 3.0724 MHz
#define GPIO_I2S_RCLK GPIO_PIN_23	// RCLK_O/DI 24.5792 MHz
#define GPIO_I2S_DI   GPIO_PIN_2	// DIO/DO
#define GPIO_I2S_FS   GPIO_PIN_1	// FS 48.0062 kHz
#endif

#define USE_POW_ON_OFF			0 // =1 Power-Down Control in Start/Stop, =0 Power-Down Control WakeUp/Sleep

static struct bflb_device_s *i2s0;
static struct bflb_device_s *dma_i2s;
static struct bflb_device_s *mgpio;

i2s_cfg_t i2s_cfg = {
		.freq = 24576000,
		.wakeupcnt = 512,
		.channels = 3,
		.start = 0
};

#define I2S_RX_BUF_LEN	32
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint32_t i2s_rx_buffer[I2S_RX_BUF_LEN];


static void i2s_set_channels(struct bflb_device_s *dev, uint8_t channels) {
	uint32_t regval;
	uint32_t reg_base = dev->reg_base;
#if 0
//	rdiv = (24576000 / 2 * 10 / (48000*32*2) + 5) / 10 - 1; = 3
    /* integer frequency segmentation by rounding */

    /* bclk timing config */
    regval = getreg32(reg_base + I2S_BCLK_CONFIG_OFFSET);
    regval &= ~I2S_CR_BCLK_DIV_L_MASK;
    regval &= ~I2S_CR_BCLK_DIV_H_MASK;
    regval |= 3 << I2S_CR_BCLK_DIV_L_SHIFT;
    regval |= 3 << I2S_CR_BCLK_DIV_H_SHIFT;
    putreg32(regval, reg_base + I2S_BCLK_CONFIG_OFFSET);
#endif

	regval = getreg32(reg_base + I2S_CONFIG_OFFSET);
	regval &= ~I2S_CR_MONO_RX_CH;
	regval &= ~I2S_CR_MONO_MODE;
	regval |= I2S_CR_FS_1T_MODE;
	regval &= ~I2S_CR_FS_CH_CNT_MASK;
	switch(channels) {
	case 1: // Mono mode L chnl
		regval |= I2S_CR_MONO_RX_CH;
		regval |= I2S_CR_MONO_MODE;
		regval |= 1 << I2S_CR_FS_CH_CNT_SHIFT;
		break;
	case 2: // Mono mode R chnl
		regval |= I2S_CR_MONO_MODE;
		regval |= 1 << I2S_CR_FS_CH_CNT_SHIFT;
		break;
	default: // Stereo mode L & R chnl
		regval |= 1 << I2S_CR_FS_CH_CNT_SHIFT;
		break;
	}
	putreg32(regval, reg_base + I2S_CONFIG_OFFSET);
}
// ATTR_TCM_SECTION
static ATTR_TCM_SECTION void dma_i2s_isr(void *arg) {
#if 1 // One channel
	if(i2s_cfg.start == 0)
		return;
	if(i2s_cfg.wakeupcnt) {
		i2s_cfg.wakeupcnt--;
	} else {
		data_buffer_wr_ptr &= DATA_BUF_CNT-1;
		for(int i = 0; i < I2S_RX_BUF_LEN; i++) {
			data_buffer[data_buffer_wr_ptr++] = i2s_rx_buffer[i];
		}
		data_buffer_wr_ptr &= DATA_BUF_CNT-1;
	#else // 2 channels
		memcpy(&data_buffer[data_buffer_wr_ptr], i2s_rx_buffer, sizeof(i2s_rx_buffer));
		data_buffer_wr_ptr += I2S_RX_BUF_LEN;
		data_buffer_wr_ptr &= DATA_BUF_CNT-1;
	#endif
	}
}
#if 1
#include "bl702_common.h"
#include "pds_reg.h"

/*
 *  0x374BC6, 12288000 HZ / 36
	0x32CCED, 11289600 HZ / 36  ?
	0x32CCED,  5644800 HZ / 72  ?
	0x6E978D, 24576000 HZ / 36
	0x6C0000, 24000000 HZ / 36
	0x3E8000, 50000000 HZ / 36
 *
 * 15381500 15360000
 * 24578500 24576000
 */
BL_Err_Type Set_Audio_PLL_Freq(uint32_t freq_hz) {

	uint32_t tmpVal;
	uint32_t div = 36;
	uint32_t prediv = 2;
	union {
		uint64_t dd;
		uint32_t dw[2];
	}x;

	if (freq_hz <= 8000*512) {
		if (freq_hz <= 2500*512) { // 1.28 MHz
			prediv = 8;
			div = 108;
			x.dd = 0x389F83BE4ul;
		} else if (freq_hz <= 4000*512) { // 1.28..2.048 MHz
			prediv = 4;
			div = 108;
			x.dd = 0x1C4FC1DF2ul;
		} else { 				// 2.048..4.096 MHz
			div = 108;
			x.dd = 0xE27E0EF9ul;
		}
	} else if (freq_hz <= 19531*512) { // 4.096..9.9999 MHz
		div = 72;
		x.dd = 0x96FEB4A6ul;
	} else if (freq_hz <= 66000*512) { // 9.9999..33.792 MHz
		div = 36;
		x.dd = 0x4B7F5A53ul;
	} else { // to 100000 sps (51.2 MHz)
		div = 18;
		x.dd = 0x25BFAD2Aul;
	}
	x.dd = x.dd * freq_hz + 0x80000000ul;

    /*set PDS_CLKPLL_REFDIV_RATIO as 0x2 */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CLKPLL_TOP_CTRL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CLKPLL_REFDIV_RATIO, prediv);
    BL_WR_REG(PDS_BASE, PDS_CLKPLL_TOP_CTRL, tmpVal);

    /*set clkpll_sdmin as sdmin*/
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CLKPLL_SDM);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CLKPLL_SDMIN, x.dw[1]);

    BL_WR_REG(PDS_BASE, PDS_CLKPLL_SDM, tmpVal);

    /*reset pll */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_PU_RST_CLKPLL);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_PU_CLKPLL_SFREG, 1);
    BL_WR_REG(PDS_BASE, PDS_PU_RST_CLKPLL, tmpVal);

    printf("SYSCLK: %u Hz\r\n", freq_hz);

    BL702_Delay_MS(10);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_PU_CLKPLL, 1);
    BL_WR_REG(PDS_BASE, PDS_PU_RST_CLKPLL, tmpVal);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CLKPLL_RESET_FBDV, 1);
    BL_WR_REG(PDS_BASE, PDS_PU_RST_CLKPLL, tmpVal);

    printf("CLKPLL: 0x%08x, POSTDIV: 0x%02x\r\n", x.dw[1], div);

    BL702_Delay_MS(10);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CLKPLL_RESET_FBDV, 0);
    BL_WR_REG(PDS_BASE, PDS_PU_RST_CLKPLL, tmpVal);

    /*set div for audio pll */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CLKPLL_TOP_CTRL);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CLKPLL_POSTDIV, div);

    BL_WR_REG(PDS_BASE, PDS_CLKPLL_TOP_CTRL, tmpVal);

    return SUCCESS;
}
#endif

#if 0 // выводы переключения на PCM1802 не сделаны
// Set Sampling Frequency PCM1802
// 32, 44.1, 48, 64, 88.2, 96 kHz
int	Set_Sampling_Frequency_PCM1802(int smpr_type) {
/* OSR:  OSR controls the oversampling ratio of the delta-sigma modulator, ×64 or ×128.
 0 x64
 1 ×128 */

/*	MODE1:MODE0
 0 0 Slave mode (256 fS, 384 fS, 512 fS, 768 fS)
 0 1 Master mode (512 fS)
 1 0 Master mode (384 fS)
 1 1 Master mode (256 fS) */

	24576000/512 = 48000
	24576000/384 = 64000
	24576000/256 = 96000

}
#endif

/* i2s gpio init */
static void i2s_gpio_init(void) {
    mgpio = bflb_device_get_by_name("gpio");

#ifdef GPIO_I2S_POW
    bflb_gpio_reset(mgpio, GPIO_I2S_POW);
    bflb_gpio_init(mgpio, GPIO_I2S_POW, GPIO_OUTPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_1);
    bflb_gpio_reset(mgpio, GPIO_I2S_POW);
#endif

    /* I2S_DI */
    bflb_gpio_init(mgpio, GPIO_I2S_DI, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_BCLK */
    bflb_gpio_init(mgpio, GPIO_I2S_BCLK, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_FS */
    bflb_gpio_init(mgpio, GPIO_I2S_FS, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_RCLK */
    bflb_gpio_init(mgpio, GPIO_I2S_RCLK, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

static struct bflb_dma_channel_lli_pool_s rx_llipool[1];
static struct bflb_dma_channel_lli_transfer_s rx_transfers[1];

static void i2s_dma_init(struct bflb_device_s *i2sx) {
    static const struct bflb_dma_channel_config_s rx_config = {
        .direction = DMA_PERIPH_TO_MEMORY,
        .src_req = DMA_REQUEST_I2S_RX,
        .dst_req = DMA_REQUEST_NONE,
        .src_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_32BIT,
        .dst_width = DMA_DATA_WIDTH_32BIT,
    };

    printf("I2S: DMA init...\r\n");
    /* enable dma */
    bflb_i2s_link_rxdma(i2sx, true);

    dma_i2s = bflb_device_get_by_name("dma0_ch1");
    if(!dma_i2s) {
    	printf("I2S: Not dev dma0_ch1!\r\n");
    	return;
    }
    bflb_dma_channel_init(dma_i2s, &rx_config);
    bflb_dma_channel_tcint_clear(dma_i2s);
    bflb_dma_channel_irq_attach(dma_i2s, dma_i2s_isr, NULL);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)i2s_rx_buffer;
    rx_transfers[0].nbytes = sizeof(i2s_rx_buffer);

    // dma lli init
    uint32_t num = bflb_dma_channel_lli_reload(dma_i2s, rx_llipool, 1, rx_transfers, 1);
    bflb_dma_channel_lli_link_head(dma_i2s, rx_llipool, num);
}

/*
static void i2s_chl_init(struct bflb_device_s *i2sx) {
    if(i2s_cfg.channels == 3) {
        // sync L/R shannel in stereo
    	i2s_set_channels(i2sx, 0);
    } else
    	i2s_set_channels(i2sx, i2s_cfg.channels);
}
*/
static void reset_i2s0(void) {
	// PERIPHERAL_CLOCK_I2S_ENABLE();
	uint32_t tmpVal = getreg32(BFLB_GLB_CGEN1_BASE);
	tmpVal |= (1 << 26);
    putreg32(tmpVal, BFLB_GLB_CGEN1_BASE);

	tmpVal = getreg32(GLB_BASE+GLB_SWRST_CFG1_OFFSET);
	tmpVal |= GLB_SWRST_S1AA_MSK; // software reset I2S
	putreg32(tmpVal, GLB_BASE + GLB_SWRST_CFG1_OFFSET);
	tmpVal &= ~GLB_SWRST_S1AA_MSK; // software reset I2S
	putreg32(tmpVal, GLB_BASE + GLB_SWRST_CFG1_OFFSET);
}

/* init I2S slave rx*/
static void I2S_init(uint8_t channels, uint32_t freq) {

	static const struct bflb_i2s_config_s i2s_scfg = {
        .bclk_freq_hz = 48000*32*2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_SLAVE, // I2S_ROLE_MASTER
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_2, // stereo/mono mode
        .frame_width = I2S_SLOT_WIDTH_32,
        .data_width = I2S_SLOT_WIDTH_24,
        .fs_offset_cycle = 1, // =1 LRCK, =0 FSY PCM1802 (Master,fmt1=0,fmt0=1,mode1=0,mode0=1,osr=0,bypas=1)

        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0
    };

	i2s_cfg.start = 0;

	data_buffer_wr_ptr = 0;
	data_buffer_rd_ptr = 0;

	i2s_cfg.channels = channels & 3;
	if(!i2s_cfg.channels)
		i2s_cfg.channels = 1;

    printf("I2S: Init...\r\n");

    /* Set work audio clock (SCK) */
    PDS_Enable_PLL_Clk(PDS_PLL_CLK_48M);
    if(i2s_cfg.freq != freq) {
    	if (freq < 1000*512) // min ?
    		i2s_cfg.freq = 1000*512;
    	else if (freq > 96000000) // max у PCM1802 49.152 MHz, ограничим до 96 MHz
    		i2s_cfg.freq = 96000000;
    	else
    		i2s_cfg.freq = freq;
      	Set_Audio_PLL_Freq(i2s_cfg.freq);
    }
    GLB_Set_I2S_CLK(ENABLE, GLB_I2S_OUT_REF_CLK_SRC);

    reset_i2s0();
    /* i2s init */
    i2s0 = bflb_device_get_by_name("i2s0");
    if(!i2s0) {
    	printf("I2S: Not dev i2s0!\r\n");
    	return;
    }
    bflb_i2s_init(i2s0, &i2s_scfg);
  	i2s_set_channels(i2s0, i2s_cfg.channels);
    i2s_dma_init(i2s0);

    i2s_cfg.wakeupcnt = 512;
}


void I2S_Start(uint8_t channels, uint32_t freq) {
	if(!i2s0) {
		printf("I2S: Not init!\r\n");
		return;
	}
	if(i2s_cfg.start == 0 || i2s_cfg.channels != channels || i2s_cfg.freq != freq) {
		printf("I2S: ReInit\r\n");
	    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, 0);
		bflb_dma_channel_stop(dma_i2s);
	    bflb_i2s_deinit(i2s0);
		bflb_gpio_reset(mgpio, GPIO_I2S_POW);
		I2S_init(channels, freq);
		bflb_dma_channel_start(dma_i2s);
		bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_RX);
		bflb_gpio_set(mgpio, GPIO_I2S_POW);
	} else if (i2s_cfg.start) {
		printf("I2S: Already Started\r\n");
	} else {
		printf("I2S: ReStart\r\n");
	}
	blk_read_count = 0;
	blk_overflow_cnt = 0;
    i2s_cfg.start = 1;
}

void I2S_Stop(void) {
	if(!i2s0) {
		printf("I2S: Not init!\r\n");
		return;
	}
	if(i2s_cfg.start) {
		printf("I2S: Stop\r\n");
	} else {
		printf("I2S: Already Stoped\r\n");
	}
	i2s_cfg.start = 0;
	data_buffer_wr_ptr = 0;
	data_buffer_rd_ptr = 0;
}

void I2S_Sleep(void) {
	if(i2s0) {
		bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, 0);
		bflb_dma_channel_stop(dma_i2s);
		bflb_i2s_deinit(i2s0);
		GLB_Set_I2S_CLK(DISABLE, GLB_I2S_OUT_REF_CLK_SRC);
		bflb_gpio_reset(mgpio, GPIO_I2S_POW);
		i2s_cfg.start = 0;
		data_buffer_wr_ptr = 0;
		data_buffer_rd_ptr = 0;
		i2s0 = NULL;
		printf("I2S: Sleep\r\n");
	}
}

void I2S_WakeUp(void) {
	if(!i2s0) {
		uint32_t freq = i2s_cfg.freq;
		i2s_cfg.freq = 0;
		I2S_init(i2s_cfg.channels, freq);
	    /* gpio init */
	    i2s_gpio_init();
		bflb_gpio_set(mgpio, GPIO_I2S_POW);
	    i2s_cfg.wakeupcnt = 512;
	}
}


#endif // USE_I2S
