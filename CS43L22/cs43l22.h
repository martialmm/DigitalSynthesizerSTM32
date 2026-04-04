/*
 * cs43l22.h
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#ifndef CS43L22_H_
#define CS43L22_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

// MACROS
#define CS43_OPERATION_CHECK(operation) do{ HAL_StatusTypeDef status = (operation); \
											if(status != HAL_OK) return status; \
										}while(0);
// REGISTERS
#define REG_INIT_00				0x00
#define REG_INIT_47				0x47
#define REG_INIT_32				0x32
#define REG_POWER_CTRL_1		0x02
#define REG_POWER_CTRL_2		0x04
#define REG_CLOCKING_CTRL 		0x05
#define REG_INTERFACE_CTRL_1	0x06
#define REG_PLAYBACK_CTRL_1 	0x0D
#define REG_PLAYBACK_CTRL_2 	0x0F
#define REG_MASTER_A_VOL		0x20
#define REG_MASTER_B_VOL		0x21
#define REG_HEADPHONE_A_VOL		0x22
#define REG_HEADPHONE_B_VOL		0x23
#define REG_SPEAKER_VOL			0X24
#define REG_MISC_CTRL			0x0E
#define REG_PCM_A_VOL			0x1A
#define REG_PCM_B_VOL			0x1B

// volume defines
#define MASTER_MIN_GAIN		-102.0f
#define MASTER_MAX_GAIN		12.0f
#define PCM_MIN_GAIN		-51.5f
#define PCM_MAX_GAIN		12.0f
#define HEADPHONE_MAX_VOL	100
#define HEADPHONE_MIN_VOL	0

typedef enum{
	MUTED,
	UNMUTED
}outputState;

// settings for startup
typedef struct{
	//public
	GPIO_TypeDef*	resetPort;
	uint8_t			resetPin;
}CS43L22_InitTypeDef;

// settings for usage
typedef struct{
	// public
	I2C_HandleTypeDef*   i2c;
	I2S_HandleTypeDef*   i2s;
	CS43L22_InitTypeDef  Init;
	uint8_t deviceAddress;

	// private
	uint8_t speakerVolume; // reserved for future use
	uint8_t headphoneVolume;
	outputState speakerState;
	outputState headphoneState;
}CS43L22_HandleTypeDef;

// public functions
HAL_StatusTypeDef CS43L22_Initialization(CS43L22_HandleTypeDef* cs43l22);
HAL_StatusTypeDef CS43L22_Reset(CS43L22_HandleTypeDef* cs43l22);
HAL_StatusTypeDef setHeadphoneVolume(CS43L22_HandleTypeDef* cs43l22, uint8_t targetVolume);
HAL_StatusTypeDef muteheadphoneOutput(CS43L22_HandleTypeDef* cs43l22);
HAL_StatusTypeDef unmuteHeadphoneOutput(CS43L22_HandleTypeDef* cs43l22);
HAL_StatusTypeDef muteAllOutputs(CS43L22_HandleTypeDef* cs43l22);
HAL_StatusTypeDef unmuteAllOutputs(CS43L22_HandleTypeDef* cs43l22);
uint8_t getHeadphoneVolume(CS43L22_HandleTypeDef* cs43l22);
uint8_t getSpeakerVolume(CS43L22_HandleTypeDef* cs43l22);
outputState getHeadphoneOutputState(CS43L22_HandleTypeDef* cs43l22);
outputState getSpeakerOutputState(CS43L22_HandleTypeDef* cs43l22);

#endif /* CS43L22_H_ */
