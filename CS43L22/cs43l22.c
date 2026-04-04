/*
 * cs43l22.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "cs43l22.h"

// private functions
static HAL_StatusTypeDef writeToRegister(CS43L22_HandleTypeDef* cs43l22, uint16_t registerAddress, uint8_t* datasToWrite);
static HAL_StatusTypeDef readRegister(CS43L22_HandleTypeDef* cs43l22, uint16_t registerAddress, uint8_t* datasRead);
static HAL_StatusTypeDef wakeupDevice(CS43L22_HandleTypeDef *cs43l22);
static HAL_StatusTypeDef initializeRequiredRegisters(CS43L22_HandleTypeDef *cs43l22);
static HAL_StatusTypeDef configureClock(CS43L22_HandleTypeDef *cs43l22);
static HAL_StatusTypeDef configureI2SInterface(CS43L22_HandleTypeDef *cs43l22);
static HAL_StatusTypeDef setPCMVolumeForAllChannels(CS43L22_HandleTypeDef *cs43l22, float targetGaindB);
static HAL_StatusTypeDef setMasterGainVolume(CS43L22_HandleTypeDef* cs43l22, float targetGaindB);
static HAL_StatusTypeDef powerHeadphoneOnly(CS43L22_HandleTypeDef* cs43l22);
static HAL_StatusTypeDef enableDigitalSoftRampAndZeroCross(CS43L22_HandleTypeDef* cs43l22);
static HAL_StatusTypeDef disableDigitalSoftRampAndZeroCross(CS43L22_HandleTypeDef* cs43l22);


// ---------- PUBLIC FUNCTIONS ---------- //

HAL_StatusTypeDef CS43L22_Initialization(CS43L22_HandleTypeDef* cs43l22){
	uint8_t datasToWrite;

	// ----- POWER-UP SEQUENCE -----

	// 1)
	CS43_OPERATION_CHECK(wakeupDevice(cs43l22));

	// 2) power-down
	datasToWrite = 0x01;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_POWER_CTRL_1, &datasToWrite));

	// 3) Nothing to do

	// 4) Required initialization settings
	CS43_OPERATION_CHECK(initializeRequiredRegisters(cs43l22));

	// 5) configure clock settings
	CS43_OPERATION_CHECK(configureClock(cs43l22));

	// Interface control 1 => not in recommended sequence but have to do it
	CS43_OPERATION_CHECK(configureI2SInterface(cs43l22));

	// PCM A & B volume at 0dB
	CS43_OPERATION_CHECK(setPCMVolumeForAllChannels(cs43l22, 0.0f));

	// Master Volume at 0dB
	CS43_OPERATION_CHECK(setMasterGainVolume(cs43l22, 0.0f));

	// enable digital soft ramp and zero cross
	CS43_OPERATION_CHECK(enableDigitalSoftRampAndZeroCross(cs43l22));

	// Headphone A & B channels are always ON
	// Speaker A & B channels are always OFF
	CS43_OPERATION_CHECK(powerHeadphoneOnly(cs43l22));

	// 6) Set power_control_1 at 0x9E for powerup
	datasToWrite = 0x9E;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_POWER_CTRL_1, &datasToWrite));

	return HAL_OK;
}

HAL_StatusTypeDef CS43L22_Reset(CS43L22_HandleTypeDef* cs43l22){

	// ----- POWER-DOWN SEQUENCE ----- //

	uint8_t datasToWrite;

	// 1) Mute headphone and speaker
	CS43_OPERATION_CHECK(muteAllOutputs(cs43l22));

	// 2) disable soft ramp and zero cross transitions
	CS43_OPERATION_CHECK(disableDigitalSoftRampAndZeroCross(cs43l22));

	// 3) power ctrl 1 = 0x9F
	datasToWrite = 0x9F;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_POWER_CTRL_1, &datasToWrite));

	// 4) wait at least 100µs
	HAL_Delay(1);

	// 5) MCLK removed
	HAL_I2S_DMAStop(cs43l22->i2s);

	// 6) reset = low
	HAL_GPIO_WritePin(cs43l22->Init.resetPort, cs43l22->Init.resetPin, GPIO_PIN_RESET);

	return HAL_OK;
}

HAL_StatusTypeDef muteHeadphoneOutput(CS43L22_HandleTypeDef* cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_PLAYBACK_CTRL_2, &tempRegisterValueRead));
	tempRegisterValueRead |= (3 << 6);
	datasToWrite = tempRegisterValueRead;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PLAYBACK_CTRL_2, &datasToWrite));
	cs43l22->headphoneState = MUTED;

	return HAL_OK;
}

HAL_StatusTypeDef unmuteHeadphoneOutput(CS43L22_HandleTypeDef* cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_PLAYBACK_CTRL_2, &tempRegisterValueRead));
	tempRegisterValueRead &= ~(3 << 6);
	datasToWrite = tempRegisterValueRead;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PLAYBACK_CTRL_2, &datasToWrite));
	cs43l22->headphoneState = UNMUTED;

	return HAL_OK;
}

HAL_StatusTypeDef muteAllOutputs(CS43L22_HandleTypeDef* cs43l22){
    uint8_t datasToWrite;
    uint8_t tempRegisterValueRead;

    CS43_OPERATION_CHECK(readRegister(cs43l22, REG_PLAYBACK_CTRL_2, &tempRegisterValueRead));
    datasToWrite = tempRegisterValueRead | (0xF << 4); // HP + SPK
    CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PLAYBACK_CTRL_2, &datasToWrite));
    cs43l22->headphoneState = MUTED;
    cs43l22->speakerState = MUTED;

    return HAL_OK;
}

HAL_StatusTypeDef unmuteAllOutputs(CS43L22_HandleTypeDef* cs43l22){
    uint8_t datasToWrite;
    uint8_t tempRegisterValueRead;

    CS43_OPERATION_CHECK(readRegister(cs43l22, REG_PLAYBACK_CTRL_2, &tempRegisterValueRead));
    datasToWrite = tempRegisterValueRead & ~(0xF << 4);
    CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PLAYBACK_CTRL_2, &datasToWrite));
    cs43l22->headphoneState = UNMUTED;
    cs43l22->speakerState = UNMUTED;

    return HAL_OK;
}

HAL_StatusTypeDef setHeadphoneVolume(CS43L22_HandleTypeDef* cs43l22, uint8_t targetVolume){
	uint8_t volumeAttenuation = 0x01; // muted by default

	if(targetVolume > HEADPHONE_MAX_VOL) targetVolume = HEADPHONE_MAX_VOL;
	if(targetVolume < HEADPHONE_MIN_VOL) targetVolume = HEADPHONE_MIN_VOL;

	if(targetVolume > HEADPHONE_MIN_VOL){
		volumeAttenuation = (uint8_t)(-(int8_t)((HEADPHONE_MAX_VOL - targetVolume) * 128 / HEADPHONE_MAX_VOL));
	}

	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_HEADPHONE_A_VOL, &volumeAttenuation)); // HPA
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_HEADPHONE_B_VOL, &volumeAttenuation)); // HPB

	cs43l22->headphoneVolume = targetVolume;

	return HAL_OK;
}

uint8_t getHeadphoneVolume(CS43L22_HandleTypeDef* cs43l22){
	return cs43l22->headphoneVolume;
}

uint8_t getSpeakerVolume(CS43L22_HandleTypeDef* cs43l22){
	return cs43l22->speakerVolume;
}

outputState getHeadphoneOutputState(CS43L22_HandleTypeDef* cs43l22){
	return cs43l22->headphoneState;
}

outputState getSpeakerOutputState(CS43L22_HandleTypeDef* cs43l22){
	return cs43l22->speakerState;
}


// ---------- PRIVATE FUNCTIONS ---------- //

static HAL_StatusTypeDef wakeupDevice(CS43L22_HandleTypeDef *cs43l22) {
	// 1) Hold /RESET low until power supplies are stable
	HAL_GPIO_WritePin(cs43l22->Init.resetPort, cs43l22->Init.resetPin, GPIO_PIN_RESET);
	HAL_Delay(20);
	// 2) Bring /RESET high
	HAL_GPIO_WritePin(cs43l22->Init.resetPort, cs43l22->Init.resetPin, GPIO_PIN_SET);

	return HAL_OK;
}

static HAL_StatusTypeDef initializeRequiredRegisters(CS43L22_HandleTypeDef *cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	datasToWrite = 0x99;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_00, &datasToWrite));

	datasToWrite = 0x80;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_47, &datasToWrite));

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_INIT_32, &tempRegisterValueRead));
	tempRegisterValueRead |= (1 << 7);
	datasToWrite = tempRegisterValueRead;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_32, &datasToWrite));

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_INIT_32, &tempRegisterValueRead));
	datasToWrite = tempRegisterValueRead & ~(1 << 7);
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_32, &datasToWrite));

	datasToWrite = 0x99;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_32, &datasToWrite));

	datasToWrite = 0x00;
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INIT_00, &datasToWrite));

	return HAL_OK;
}

static HAL_StatusTypeDef configureClock(CS43L22_HandleTypeDef *cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	// auto-detect
	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_CLOCKING_CTRL, &tempRegisterValueRead));
	datasToWrite = tempRegisterValueRead | (1 << 7);
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_CLOCKING_CTRL, &datasToWrite));

	return HAL_OK;
}

static HAL_StatusTypeDef configureI2SInterface(CS43L22_HandleTypeDef *cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_INTERFACE_CTRL_1, &tempRegisterValueRead));
	datasToWrite = tempRegisterValueRead & ~(1 << 7); // set slave mode
	datasToWrite &= ~(1 << 6); // clk polarity not inverted
	datasToWrite &= ~(1 << 4); // dsp mode disabled
	datasToWrite |= (1 << 2); // dac interface format: i2s, up to 24-bit data
	datasToWrite |= (3 << 0);  // word lenght = 16 bits for i2s
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_INTERFACE_CTRL_1, &datasToWrite));

	return HAL_OK;
}

static HAL_StatusTypeDef setPCMVolumeForAllChannels(CS43L22_HandleTypeDef *cs43l22, float targetGaindB){
	if(targetGaindB > PCM_MAX_GAIN) targetGaindB =  PCM_MAX_GAIN;
	if(targetGaindB < PCM_MIN_GAIN) targetGaindB = PCM_MIN_GAIN;

	uint8_t finalGain = (uint8_t)(int8_t)(targetGaindB * 2.0f); // double cast pour complement a 2

	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PCM_A_VOL, &finalGain));
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_PCM_B_VOL, &finalGain));

	return HAL_OK;
}

static HAL_StatusTypeDef setMasterGainVolume(CS43L22_HandleTypeDef* cs43l22, float targetGaindB){
	if(targetGaindB > MASTER_MAX_GAIN) targetGaindB =  MASTER_MAX_GAIN;
	if(targetGaindB < MASTER_MIN_GAIN) targetGaindB = MASTER_MIN_GAIN;

	uint8_t finalGain = (uint8_t)(int8_t)(targetGaindB * 2.0f); // double cast pour complement a 2

	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_MASTER_A_VOL, &finalGain));
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_MASTER_B_VOL, &finalGain));

	return HAL_OK;
}

static HAL_StatusTypeDef enableDigitalSoftRampAndZeroCross(CS43L22_HandleTypeDef* cs43l22){
    uint8_t datasToWrite;
    uint8_t tempRegisterValueRead;

    CS43_OPERATION_CHECK(readRegister(cs43l22, REG_MISC_CTRL, &tempRegisterValueRead));
    datasToWrite = tempRegisterValueRead | (1 << 0);
    datasToWrite |= (1 << 1);
    CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_MISC_CTRL, &datasToWrite));

    return HAL_OK;
}

static HAL_StatusTypeDef disableDigitalSoftRampAndZeroCross(CS43L22_HandleTypeDef* cs43l22){
	uint8_t datasToWrite;
	uint8_t tempRegisterValueRead;

	CS43_OPERATION_CHECK(readRegister(cs43l22, REG_MISC_CTRL, &tempRegisterValueRead));
	datasToWrite = tempRegisterValueRead & ~(1 << 0);
	datasToWrite &= ~(1 << 1);
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_MISC_CTRL, &datasToWrite));

	return HAL_OK;
}


static HAL_StatusTypeDef powerHeadphoneOnly(CS43L22_HandleTypeDef* cs43l22){
	uint8_t datasToWrite = 0x00;

	datasToWrite |= (2 << 6);  // SPK A : always off
	datasToWrite |= (2 << 4);  // SPK B : always off
	datasToWrite |= (3 << 2);  // HP A  : always on
	datasToWrite |= (3 << 0);  // HP B  : always on
	CS43_OPERATION_CHECK(writeToRegister(cs43l22, REG_POWER_CTRL_2, &datasToWrite));

	return HAL_OK;
}

static HAL_StatusTypeDef writeToRegister(CS43L22_HandleTypeDef* cs43l22, uint16_t registerAddress, uint8_t* datasToWrite){
	CS43_OPERATION_CHECK(HAL_I2C_Mem_Write(cs43l22->i2c, cs43l22->deviceAddress, registerAddress, I2C_MEMADD_SIZE_8BIT, datasToWrite, 1, HAL_MAX_DELAY));

	return HAL_OK;
}

static HAL_StatusTypeDef readRegister(CS43L22_HandleTypeDef* cs43l22, uint16_t registerAddress, uint8_t* datasRead){
	CS43_OPERATION_CHECK(HAL_I2C_Mem_Read(cs43l22->i2c, cs43l22->deviceAddress, registerAddress, I2C_MEMADD_SIZE_8BIT, datasRead, 1, HAL_MAX_DELAY));

	return HAL_OK;
}
