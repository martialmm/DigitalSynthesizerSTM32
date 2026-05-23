/*
 * audio.c
 *
 *  Created on: May 15, 2026
 *      Author: mars
 */

#include "main.h"
#include "audio.h"
#include "oscillator.h"

static void feedDMAAudioBuffer(int16_t* buffer, uint16_t num_frames);

static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
static Oscillator_t* oscillator = NULL;

void oscillatorRegister(Oscillator_t* oscillatorToAdd) {
	oscillator = oscillatorToAdd;
}

void startI2SOscillator(I2S_HandleTypeDef* hi2s){
	HAL_I2S_Transmit_DMA(hi2s, (uint16_t*) &dmaAudioBuffer, TOTAL_BUFFER_SIZE);
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&dmaAudioBuffer[TOTAL_BUFFER_SIZE >> 1], NUMBER_OF_FRAMES_PER_HALF);
}

static void feedDMAAudioBuffer(int16_t* buffer, uint16_t num_frames){
	const float volumeSmoothing = 0.001f;

	for(uint16_t i = 0; i < num_frames; i++){
		float oscillatorCurrentVolume = getOscillatorCurrentVolume(oscillator);
		float oscillatorTargetVolume = getOscillatorTargetVolume(oscillator);

		oscillatorCurrentVolume = oscillatorCurrentVolume + (oscillatorTargetVolume - oscillatorCurrentVolume) * volumeSmoothing;
		setOscillatorCurrentvolume(oscillator, oscillatorCurrentVolume);

		float sample = getNextOscillatorSample(oscillator) * getOscillatorCurrentVolume(oscillator); // * getEnveloppe()

		// securite pour pas perde un ou deux tympans
		if (sample > 32767.0f) sample = 32767.0f;
		if (sample < -32768.0f) sample = -32768.0f;

		buffer[2*i] = (int16_t) sample;
		buffer[2*i+1] = (int16_t) sample;
	}
}
