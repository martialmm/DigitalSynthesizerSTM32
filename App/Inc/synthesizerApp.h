/*
 * synthesizerApp.h
 *
 *  Created on: Apr 21, 2026
 *      Author: mars
 */

#ifndef INC_SYNTHESIZERAPP_H_
#define INC_SYNTHESIZERAPP_H_

#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef* ch_hadc1;
extern I2C_HandleTypeDef* ch_hi2c1;
extern I2S_HandleTypeDef* i2sForExternalDAC;
extern DMA_HandleTypeDef* ch_hdma_spi2_tx;
extern TIM_HandleTypeDef* timerForUserInputsScan;

void synthesizer();

#endif /* INC_SYNTHESIZERAPP_H_ */
