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

#if BOARD == bl702rd
#define GPIO_I2S_POW  GPIO_PIN_24
//#define GPIO_I2S_SCLK GPIO_PIN_24
#define GPIO_I2S_BCLK GPIO_PIN_28	// BCLK 3.0724 MHz
#define GPIO_I2S_RCLK GPIO_PIN_27	// RCLK_O/DI 24.5792 MHz
#define GPIO_I2S_DI   GPIO_PIN_26
#define GPIO_I2S_FS   GPIO_PIN_1	// FS 48.0062 kHz
#else
#define GPIO_I2S_POW  GPIO_PIN_25	//
//#define GPIO_I2S_SCLK GPIO_PIN_25	// SCK 24.45792 MHz
#define GPIO_I2S_BCLK GPIO_PIN_28	// BCLK 3.0724 MHz
#define GPIO_I2S_RCLK GPIO_PIN_27	// RCLK_O/DI 24.5792 MHz
#define GPIO_I2S_DI   GPIO_PIN_2	// DIO/DO
#define GPIO_I2S_FS   GPIO_PIN_1	// FS 48.0062 kHz
#endif

static struct bflb_device_s *i2s0;
static struct bflb_device_s *dma_i2s;
static struct bflb_device_s *mgpio;


#define I2S_RX_BUF_LEN	32
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint32_t i2s_rx_buffer[I2S_RX_BUF_LEN];



static void dma_i2s_isr(void *arg) {
#if 1
	int i;
	//data_buffer_wr_ptr &= DATA_BUF_CNT-1;
	for(i = 0; i < I2S_RX_BUF_LEN; i+=2) {
		data_buffer[data_buffer_wr_ptr++] = i2s_rx_buffer[i];
	}
	data_buffer_wr_ptr &= DATA_BUF_CNT-1;
#else
	memcpy(&data_buffer[data_buffer_wr_ptr], i2s_rx_buffer, sizeof(i2s_rx_buffer));
	data_buffer_wr_ptr += I2S_RX_BUF_LEN;
	data_buffer_wr_ptr &= DATA_BUF_CNT-1;
#endif
}

static void i2s_gpio_init(void) {

    mgpio = bflb_device_get_by_name("gpio");
    /* I2S_FS */
    bflb_gpio_init(mgpio, GPIO_I2S_FS, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_DI */
    bflb_gpio_init(mgpio, GPIO_I2S_DI, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_RCLK */
    bflb_gpio_init(mgpio, GPIO_I2S_RCLK, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_BCLK */
    bflb_gpio_init(mgpio, GPIO_I2S_BCLK, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
#ifdef GPIO_I2S_SCLK
    /* MCLK CLKOUT */
    bflb_gpio_init(mgpio, GPIO_I2S_SCLK, GPIO_FUNC_CLK_OUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
#endif
#ifdef GPIO_I2S_POW
    bflb_gpio_init(mgpio, GPIO_I2S_POW, GPIO_OUTPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_1);
    bflb_gpio_reset(mgpio, GPIO_I2S_POW);
#endif
}

static struct bflb_dma_channel_lli_pool_s rx_llipool[1];
static struct bflb_dma_channel_lli_transfer_s rx_transfers[1];

static void I2S_dma_init(void) {

    struct bflb_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 48000*32*2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_SLAVE, // I2S_ROLE_SLAVE,
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_2, // stereo
        .frame_width = I2S_SLOT_WIDTH_32,
        .data_width = I2S_SLOT_WIDTH_24,
        .fs_offset_cycle = 1,

        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0,
    };

    struct bflb_dma_channel_config_s rx_config = {
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

    printf("I2S: Init...\r\n");
    /* gpio init */
    i2s_gpio_init();

    /* mclk clkout init */
    PDS_Set_Audio_PLL_Freq(AUDIO_PLL_50000000_HZ);// AUDIO_PLL_24576000_HZ); // AUDIO_PLL_24576000_HZ);
    GLB_Set_I2S_CLK(ENABLE, GLB_I2S_OUT_REF_CLK_SRC);
#ifdef GPIO_I2S_SCLK
#if (GPIO_I2S_SCLK & 1)
    GLB_Set_Chip_Out_1_CLK_Sel(GLB_CHIP_CLK_OUT_I2S_REF_CLK);
#else
    GLB_Set_Chip_Out_0_CLK_Sel(GLB_CHIP_CLK_OUT_I2S_REF_CLK); // GLB_CHIP_CLK_OUT_AUDIO_PLL_CLK, GLB_CHIP_CLK_OUT_XTAL_SOC_32M
#endif
#endif

    /* i2s init */
    i2s0 = bflb_device_get_by_name("i2s0");
    if(!i2s0) {
    	printf("I2S: Not dev i2s0!\r\n");
    	return;
    }
    bflb_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    //bflb_i2s_link_txdma(i2s0, true);
    bflb_i2s_link_rxdma(i2s0, true);

    printf("I2S: DMA init...\r\n");
    dma_i2s = bflb_device_get_by_name("dma0_ch1");
    if(!dma_i2s) {
    	printf("I2S: Not dev dma0_ch1!\r\n");
    	return;
    }
    bflb_dma_channel_init(dma_i2s, &rx_config);
    bflb_dma_channel_irq_attach(dma_i2s, dma_i2s_isr, NULL);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)i2s_rx_buffer;
    rx_transfers[0].nbytes = sizeof(i2s_rx_buffer);

    //printf("I2S: dma lli init...\r\n");
    uint32_t num = bflb_dma_channel_lli_reload(dma_i2s, rx_llipool, 1, rx_transfers, 1);
    bflb_dma_channel_lli_link_head(dma_i2s, rx_llipool, num);
    //printf("I2S: dma lli num: %d \r\n", num);
//    bflb_dma_channel_start(dma_i2s);

//    printf("I2S: enable i2s rx\r\n", num);
//    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_RX);
}

void I2S_Start(void) {
	if(!i2s0) {
		printf("I2S: Not init!\r\n");
		return;
	}
	printf("I2S: Start\r\n");
    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, 0);
	bflb_dma_channel_stop(dma_i2s);
#ifdef GPIO_I2S_POW
	bflb_gpio_set(mgpio, GPIO_I2S_POW);
#endif
	data_buffer_rd_ptr = 0;
	data_buffer_wr_ptr = 0;
    bflb_i2s_feature_control(i2s0, I2S_CMD_CLEAR_RX_FIFO, 0);
    bflb_dma_channel_start(dma_i2s);
    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_RX);
}

void I2S_Stop(void) {
	if(!i2s0) {
		printf("I2S: Not init!\r\n");
		return;
	}
    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, 0);
	bflb_dma_channel_stop(dma_i2s);
#ifdef GPIO_I2S_POW
	bflb_gpio_reset(mgpio, GPIO_I2S_POW);
#endif
	//bflb_i2s_deinit(i2s0);
	printf("I2S: Stop\r\n");
}

void I2S_Sleep(void) {
	if(i2s0) {
		I2S_Stop();
		bflb_i2s_deinit(i2s0);
#ifdef GPIO_I2S_POW
		bflb_gpio_reset(mgpio, GPIO_I2S_POW);
#endif
		i2s0 = NULL;
		printf("I2S: Sleep\r\n");
	}
}

void I2S_WakeUp(void) {
	if(!i2s0) {
		I2S_dma_init();
	}
}


#endif // USE_I2S
