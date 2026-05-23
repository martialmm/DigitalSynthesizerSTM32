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

static const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform);

struct Oscillator {
    float frequency;
    float targetVolume;
    float currentVolume;
    uint32_t phase;
    uint32_t phaseIncrement;
    const int16_t* activeLookupTable;
    int8_t detune;
    Waveform_t waveform;
};

I2C_HandleTypeDef* ch_hi2c1 = NULL;
DMA_HandleTypeDef* ch_hdma_spi2_tx = NULL;


// ---- INITIALIZATION ---- //

Oscillator_t* createOscillator(void) {
    static Oscillator_t instance = {0};
    return &instance;
}

void initOscillator(Oscillator_t* oscillator){
	oscillator->activeLookupTable = sineLookupTable;
	oscillator->detune = 0;
	oscillator->frequency = 0.0f;
	oscillator->phase = 0;
	oscillator->phaseIncrement = 0;
	oscillator->targetVolume = 0.0f;
	oscillator->waveform = SINUS;
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

void setOscillatorTargetVolume(Oscillator_t* oscillator, float targetVolume){
	oscillator->targetVolume = targetVolume;
}

void setOscillatorCurrentvolume(Oscillator_t* oscillator, float currentVolume){
	oscillator->currentVolume = currentVolume;
}

// ---- GETTERS ---- //


float getOscillatorFrequency(Oscillator_t* oscillator){
	return oscillator->frequency;
}

float getOscillatorTargetVolume(Oscillator_t* oscillator){
	return oscillator->targetVolume;
}

float getOscillatorCurrentVolume(Oscillator_t* oscillator){
	return oscillator->currentVolume;
}


// ---- PUBLIC FUNCTIONS ---- //

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0f); // 4294967296.0 = 2^32
}

float getNextOscillatorSample(Oscillator_t* oscillator){
	float sample;

	if(oscillator->activeLookupTable != NULL) {
		sample = oscillator->activeLookupTable[oscillator->phase >> FP_SHIFT_AMOUNT];
	} else {
		sample = 0.0f;
	}

	oscillator->phase += oscillator->phaseIncrement;

	return sample;
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
