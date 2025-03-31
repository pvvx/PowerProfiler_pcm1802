/*
 * cmd_decode.c
 *
 *  Created on: 14.01.2020
 *      Author: pvvx
 */
#include "main_config.h"
#include "bl702_common.h"
#include "usb_buffer.h"
#include "i2s_drv.h"
#include "cmd_cfg.h"


#define INT_DEV_ID	0x1023  // DevID = 0x1023
#define INT_DEV_VER 0x0002  // Ver 1.2.3.4 = 0x1234


//blk_rx_pkt_t read_pkt; // приемный буфер
blk_tx_pkt_t send_pkt; // буфер отправки

uint8_t rx_len = 0; // flag - пришла команда в read_pkt
uint8_t tx_len = 0; // flag - есть данные для передачи в send_pkt

dev_adc_cfg_t cfg_adc;

/*******************************************************************************
 * Function Name : usb_ble_cmd_decode.
 * Description	 : Main loop routine.
 * Input		 : blk_tx_pkt_t * pbufo, blk_tx_pkt_t * pbufi, int rxlen
 * Return		 : txlen.
 *******************************************************************************/
unsigned int cmd_decode(blk_tx_pkt_t * pbufo, blk_rx_pkt_t * pbufi, unsigned int rxlen) {
	unsigned int txlen = 0;
//	uint8_t tmp;
//	if (rxlen >= sizeof(blk_head_t)) {
//		if (rxlen >= pbufi->head.size + sizeof(blk_head_t)) {
			pbufo->head.cmd = pbufi->head.cmd;
			switch (pbufi->head.cmd) {
			case CMD_DEV_VER: // Get Ver
				pbufo->data.ui[0] = INT_DEV_ID; // DevID = 0x1021
				pbufo->data.ui[1] = INT_DEV_VER; // Ver 1.2.3.4 = 0x1234
				txlen = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(blk_head_t);
				break;
			case CMD_DEV_CAD: // Get/Set CFG/ini ADC & Start measure
				if (pbufi->head.size) {
					memcpy(&cfg_adc, &pbufi->data.cadc,
						(pbufi->head.size > sizeof(cfg_adc))? sizeof(cfg_adc) : pbufi->head.size);
					if(pbufi->data.cadc.enable) {
						I2S_Start(cfg_adc.chnl, cfg_adc.freq);
						cfg_adc.chnl = i2s_cfg.channels;
						cfg_adc.freq = i2s_cfg.freq;
					} else
						I2S_Stop();
				}
				memcpy(&pbufo->data, &cfg_adc, sizeof(cfg_adc));
				txlen = sizeof(cfg_adc) + sizeof(blk_head_t);
				break;
			//-------
			case CMD_DEV_STA: // Status
				pbufo->data.sta.rd_cnt = blk_read_count;
				pbufo->data.sta.to_cnt = blk_overflow_cnt;
				txlen = sizeof(blk_head_t) + sizeof(dev_sta_t);
				break;
			//-------
			case CMD_DEV_DBG: // Debug
				pbufi->data.dbg.addr &= 0xfffffffc;
				int cnt;
				if(pbufi->head.size > sizeof(dev_dbg_t)) {
					cnt = pbufi->head.size - sizeof(dev_dbg_t);
					uint32_t *po = (uint32_t *)pbufi->data.dbg.addr;
					uint8_t *pi = (uint8_t *)&pbufi->data.uc[sizeof(dev_dbg_t)];
					cnt >>= 2;
					while(cnt--) {
						*po++ = pi[0] + (pi[1]<<8) + (pi[2]<<16) + (pi[3]<<24);
						pi += 4;
					}
				} else if(pbufi->head.size < sizeof(dev_dbg_t)) {
					pbufo->head.cmd |= CMD_ERR_FLG; // Error cmd
					txlen = 0 + sizeof(blk_head_t);
					break;
				}
				cnt = pbufi->data.dbg.rd_cnt;
				if(cnt){
					if(cnt > sizeof(pbufi->data)/4)
						cnt = sizeof(pbufi->data)/4;
					txlen = cnt << 2;
					uint8_t *po = (uint8_t *)pbufo->data.uc;
					uint32_t *pi = (uint32_t *)pbufi->data.dbg.addr;
					while(cnt--) {
						uint32_t r = *pi++;
						*po++ = r;
						*po++ = r>>8;
						*po++ = r>>16;
						*po++ = r>>24;
					}
				}
				txlen += sizeof(blk_head_t);
				break;
			default:
				pbufo->head.cmd |= CMD_ERR_FLG; // Error cmd
				txlen = 0 + sizeof(blk_head_t);
				break;
			};
			pbufo->head.size = txlen - sizeof(blk_head_t);
//		}
//		else
//			rxlen = 0;
//	}
	return txlen;
}
