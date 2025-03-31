/*
 * main_config.h
 *
 *  Created on: 19 нояб. 2022 г.
 *      Author: pvvx
 */

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

#define bl702dk 10
#define bl702zb 11
#define bl702rd 12

#ifndef USE_BOARD
#define USE_BOARD bl702zb
#endif

#define USE_ADC 		0

#define USE_I2S			1
#define USE_INA229 		0
//#define USE_GPIO_IRQ
//#define USE_TIMER_IRQ


#endif /* MAIN_CONFIG_H_ */
