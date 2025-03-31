/*
 * i2s_drv.h
 *
 *  Created on: 22 янв. 2023 г.
 *      Author: pvvx
 */

#ifndef I2S_DRV_H_
#define I2S_DRV_H_

typedef struct  __attribute__((packed))  _i2s_cfg_t {
	uint32_t freq;		// freq 5 000 000..50 000 000 Hz
	uint32_t wakeupcnt;	// wake-up counter
	uint8_t channels;	// 0 or 1 - Lin, 2 - Rin, 3 - Stereo
	uint8_t start; // start enabled
} i2s_cfg_t;

extern i2s_cfg_t i2s_cfg;

void I2S_Start(uint8_t channels, uint32_t freq);
void I2S_Stop(void);
void I2S_Sleep(void);
void I2S_WakeUp(void);

#endif /* I2S_DRV_H_ */
