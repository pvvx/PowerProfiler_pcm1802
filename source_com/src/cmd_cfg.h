/*
 * cmd_cfg.h
 *
 *  Created on: 14.01.2020
 *      Author: pvvx
 */

#ifndef _CMD_CFG_H_
#define _CMD_CFG_H_


// 9 RF packets: 20 + 27*8 = 236 bytes, (236-2)/2 = max 117 16-bits word
#define SMPS_BLK_CNT	116 // ((20+27*8-2)/2)&0xFFFE = 116 (9 rf-tx block)
#define DLE_DATA_SIZE	(SMPS_BLK_CNT*2+2) // [116*2+2 = 234], MTU = 234+7 = 241 ?
#define MTU_DATA_SIZE	(DLE_DATA_SIZE+7) //[max 241] -(3 байта заголовка ATT и 4 байта заголовка L2CAP)

// DEV command id:
#define CMD_DEV_VER	0x00 // Get Ver
// Status
#define CMD_DEV_STA	0x03 // Status
// ADC cfg
#define CMD_DEV_CAD	0x38 // Get/Set CFG/ini ADC & Start measure
// ADC out samples
#define CMD_DEV_EADC	0x3A // blk out regs ADC data
// Debug
#define CMD_DEV_DBG	0x0D // Debug

#define CMD_ERR_FLG	0x80 // send error cmd mask

typedef struct __attribute__((packed)) {
	uint8_t size; // размер пакета
	uint8_t cmd;  // номер команды / тип пакета
} blk_head_t;

// CMD_DEV_STA  Status
typedef struct  __attribute__((packed)) _dev_sta_t{
	uint32_t rd_cnt; // счетчик samples
	uint32_t to_cnt; // счетчик не переданных или timeout при передаче samples
} dev_sta_t;

// CMD_DEV_DBG  Debug
typedef struct  __attribute__((packed)) _dev_dbg_t{
	uint16_t rd_cnt; // счетчик байт чтения (hi byte - reserved)
	uint32_t addr; // адрес
} dev_dbg_t;


// Структура конфигурации опроса и инициализации устройства ADC
// Выходной пакет непрерывного опроса формируется по данному описанию
// CMD_DEV_CAD Get/Set CFG/ini ADC & Start measure
typedef struct __attribute__((packed)) _dev_adc_cfg_t {
	uint8_t enable;	//
	uint8_t chnl; 	// Channels
	uint32_t freq; 	// SysCLK Hz
} dev_adc_cfg_t; // [6]
extern dev_adc_cfg_t cfg_adc; // store in eep


typedef struct __attribute__((packed)) _blk_tx_pkt_t{
	blk_head_t head;
	union __attribute__((packed)) {
		uint8_t uc[DLE_DATA_SIZE-sizeof(blk_head_t)];
		int8_t sc[DLE_DATA_SIZE-sizeof(blk_head_t)];
		uint16_t ui[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint16_t)];
		int16_t si[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(int16_t)];
		uint32_t ud[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint32_t)];
		int32_t sd[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint32_t)];
		dev_sta_t sta;
		dev_adc_cfg_t cadc;
		dev_dbg_t dbg;
	} data;
} blk_tx_pkt_t;

typedef struct __attribute__((packed)) _blk_rx_pkt_t{
	blk_head_t head;
	union __attribute__((packed)) {
		uint8_t uc[DLE_DATA_SIZE-sizeof(blk_head_t)];
		int8_t sc[DLE_DATA_SIZE-sizeof(blk_head_t)];
		uint16_t ui[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint16_t)];
		int16_t si[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(int16_t)];
		uint32_t ud[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint32_t)];
		int32_t sd[(DLE_DATA_SIZE-sizeof(blk_head_t))/sizeof(uint32_t)];
		dev_adc_cfg_t cadc;
		dev_dbg_t dbg;
	} data;
} blk_rx_pkt_t;

//extern blk_rx_pkt_t read_pkt;
extern blk_tx_pkt_t send_pkt;

//extern u32 all_rd_count; // count read
//extern u32 not_send_count; // diag count

//extern u8 rx_len; // flag - пришла команда в read_pkt
//extern u8 tx_len; // flag - есть данные для передачи в send_pkt

//void send_rtm_err(u16 err_id, u16 err);

/*******************************************************************************
 * Function Name : usb_ble_cmd_decode.
 * Description	 : Main loop routine.
 * Input		 : blk_tx_pkt_t * pbufo, blk_tx_pkt_t * pbufi, int rxlen
 * Return		 : txlen.
 *******************************************************************************/
unsigned int cmd_decode(blk_tx_pkt_t * pbufo, blk_rx_pkt_t * pbufi, unsigned int rxlen);

#endif /* _CMD_CFG_H_ */
