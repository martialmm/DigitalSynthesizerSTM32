/*
 * oscillator.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include <math.h>
#include "main.h"
#include "oscillator.h"
#include "wavetables.h"


#define WAVE_AMPLITUDE 16000
#define PIPI 6.2831853f

static void feedDMAAudioBuffer(Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames);
static const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform);

static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
static Oscillator_t* oscillator1 = NULL;

struct Oscillator {
    float enveloppe;
    float frequency;
    float targetVolume;
    float currentVolume;
    uint32_t phase;
    uint32_t phaseIncrement;
    const int16_t* activeLookupTable;
    int8_t detune;
    Waveform_t waveform;
    uint8_t noteIsPlayed;
};

I2C_HandleTypeDef* ch_hi2c1 = NULL;
I2S_HandleTypeDef* i2sForExternalDAC = NULL;
DMA_HandleTypeDef* ch_hdma_spi2_tx = NULL;


// ---- INITIALIZATION ---- //

void startI2SOscillator(I2S_HandleTypeDef* hi2s){
	HAL_I2S_Transmit_DMA(hi2s, (uint16_t*) &dmaAudioBuffer, TOTAL_BUFFER_SIZE);
}

// temp for refactoring
void oscillatorRegister(Oscillator_t* oscillator) {
	oscillator1 = oscillator;
}

Oscillator_t* createOscillator(void) {
    static Oscillator_t instance = {0};
    return &instance;
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
	oscillator->noteIsPlayed = 0;
}

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0f); // 4294967296.0 = 2^32
}


// ---- SETTERS ---- //

void setOscillatorWaveform(Oscillator_t* oscillator, Waveform_t waveform) {
    if (waveform != NONE) {
    	oscillator->waveform = waveform;
    	oscillator->activeLookupTable = defineActiveLookupTableWaveform(waveform);
    }
    else{
    	oscillator->waveform = NONE;
    	oscillator->activeLookupTable = NULL;
    }
}

void setOscillatorFrequency(Oscillator_t* oscillator, float frequency){
	oscillator->frequency = frequency;
}

void setOscillatorPhaseIncrement(Oscillator_t* oscillator, uint32_t phaseIncrement){
	oscillator->phaseIncrement = phaseIncrement;
}

void setOscillatorVolume(Oscillator_t* oscillator, float targetVolume){
	oscillator->targetVolume = targetVolume;
}

void noteIsPlayed(Oscillator_t* oscillator){
	oscillator->noteIsPlayed = 1;
}

void noteIsNotPlayed(Oscillator_t* oscillator){
	oscillator->noteIsPlayed = 0;
}



// ---- GETTERS ---- //

float getOscillatorFrequency(Oscillator_t* oscillator){
	return oscillator->frequency;
}

float getOscillatorVolume(Oscillator_t* oscillator){
	return oscillator->targetVolume;
}


// ---- PRIVATE FUNCTIONS ---- //

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

	for(uint16_t i = 0; i < num_frames; i++){
		oscillator->currentVolume += (oscillator->targetVolume - oscillator->currentVolume) * volumeSmoothing;
		if(oscillator->noteIsPlayed){
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
	feedDMAAudioBuffer(oscillator1, &dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(oscillator1, &dmaAudioBuffer[TOTAL_BUFFER_SIZE >> 1], NUMBER_OF_FRAMES_PER_HALF);
}
