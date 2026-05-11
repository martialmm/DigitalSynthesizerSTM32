/*
 * oscillator.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "oscillator.h"
#include <math.h>
#include "main.h"
#include "user_interface.h"
#include "wavetables.h"

#define WAVE_AMPLITUDE 16000
#define PIPI 6.2831853f

static void feedDMAAudioBuffer(Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames);
static const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform);

static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
Oscillator_t osc1;

I2C_HandleTypeDef* ch_hi2c1 = NULL;
I2S_HandleTypeDef* i2sForExternalDAC = NULL;
DMA_HandleTypeDef* ch_hdma_spi2_tx = NULL;

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0f); // 4294967296.0 = 2^32
}

void startI2SOscillator(I2S_HandleTypeDef* hi2s){
	HAL_I2S_Transmit_DMA(hi2s, (uint16_t*) &dmaAudioBuffer, TOTAL_BUFFER_SIZE);
}

void initializeSynthesizer(){
	HAL_GPIO_WritePin(Enable_All_MUX_GPIO_Port, Enable_All_MUX_Pin, GPIO_PIN_RESET);
}

void initializeOscillator(Oscillator_t* oscillator){
	oscillator->activeLookupTable = sineLookupTable;
	oscillator->detune = 0;
	oscillator->enveloppe = 0.0f;
	oscillator->frequency = 0.0f;
	oscillator->phase = 0;
	oscillator->phaseIncrement = 0;
	oscillator->targetVolume = 0.0f;
	oscillator->waveform = SINUS;
}

void setOscillatorWaveform(Oscillator_t *osc, Waveform_t waveform) {
    if (waveform != NONE) {
        osc->waveform = waveform;
        osc->activeLookupTable = defineActiveLookupTableWaveform(waveform);
    }
    else{
        osc->waveform = NONE;
        osc->activeLookupTable = NULL;
    }
}

static const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform){
	 if(selectedWaveform == SINUS){
		 return sineLookupTable;
	 }
	else if(selectedWaveform == TRIANGLE){
		return triangleLookupTable;
	}
	else if(selectedWaveform == SAWTOOTH){
		return sawtoothLookupTable;
	}
	else{
		return squareLookupTable;
	 }
}

static void feedDMAAudioBuffer(Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames){
	float output;
	const float antipopFactor = 0.001f;
	const float volumeSmoothing = 0.001f;
	uint8_t noteButtonPressed = (userInputs.buttonsState & BTN_LOWER_OCTAVE) || (userInputs.buttonsState & BTN_UPPER_OCTAVE);

	for(uint16_t i = 0; i < num_frames; i++){
		oscillator->currentVolume += (oscillator->targetVolume - oscillator->currentVolume) * volumeSmoothing;
		if(noteButtonPressed){
			oscillator->enveloppe += antipopFactor;
			if(oscillator->enveloppe > 1.0f) oscillator->enveloppe = 1.0f;
		}
		else{
			oscillator->enveloppe -= antipopFactor;
			if (oscillator->enveloppe < 0.0f) oscillator->enveloppe = 0.0f;
		}

        if(oscillator->activeLookupTable != NULL) {
            output = oscillator->activeLookupTable[oscillator->phase >> FP_SHIFT_AMOUNT] * oscillator->enveloppe * oscillator->currentVolume;
        } else {
            output = 0.0f;
        }

		// securite pour pas perde un ou deux tympans
		if (output > 32767.0f) output = 32767.0f;
		if (output < -32768.0f) output = -32768.0f;

		buffer[2*i] = output;
		buffer[2*i+1] = output;

		oscillator->phase += oscillator->phaseIncrement;
	}
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[TOTAL_BUFFER_SIZE >> 1], NUMBER_OF_FRAMES_PER_HALF);
}
