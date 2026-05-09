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

static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
static int16_t triangleLookupTable[SAMPLE_NUMBER_LUT];
static int16_t sawtoothLookupTable[SAMPLE_NUMBER_LUT];
static int16_t squareLookupTable[SAMPLE_NUMBER_LUT];
Oscillator_t osc1;

I2C_HandleTypeDef* ch_hi2c1 = NULL;
I2S_HandleTypeDef* i2sForExternalDAC = NULL;
DMA_HandleTypeDef* ch_hdma_spi2_tx = NULL;


void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[TOTAL_BUFFER_SIZE >> 1], NUMBER_OF_FRAMES_PER_HALF);
}

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0f); // 4294967296.0 = 2^32
}

const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform){
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

void startI2SOscillator(I2S_HandleTypeDef* hi2s){
	HAL_I2S_Transmit_DMA(hi2s, (uint16_t*) &dmaAudioBuffer, TOTAL_BUFFER_SIZE);
}

void createAllLookupTables(){
	feedTriangleTable(triangleLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
	feedSawtoothTable(sawtoothLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
	feedSquareTable(squareLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
}

void initializeSynthesizer(){
	HAL_GPIO_WritePin(Enable_All_MUX_GPIO_Port, Enable_All_MUX_Pin, GPIO_PIN_RESET);
	createAllLookupTables();
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

void feedTriangleTable(int16_t* triangleLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	//we slice one period of the triangle in three equations y = ax + b
	// y --> tab[i]
	// x --> i
	// b --> wave_amplitude
	// in each calculation, I have to do the multiplication first and then the division.
	// if I do the division first, the decimal part will be lost early (because we use integers) and the error will "snowball" with the multiplication.
	// This will lead to have bad values at the extremes points (or not really precise as we want).
	// Bit shifting is used to avoid some divisions.

	const uint16_t quarterOfTheWavePeriod = tableSize >> 2;
	const uint16_t halfOfTheWavePeriod = tableSize >> 1;
	const uint16_t threeQuartersOfTheWavePeriod = quarterOfTheWavePeriod + halfOfTheWavePeriod;

	for (uint16_t i = 0; i < tableSize; i++) {
		if (i < quarterOfTheWavePeriod) {
			triangleLookupTable[i] = (waveAmplitude * i) / quarterOfTheWavePeriod;
		} else if (i < threeQuartersOfTheWavePeriod) {
			triangleLookupTable[i] = - waveAmplitude * (i - quarterOfTheWavePeriod) / quarterOfTheWavePeriod + waveAmplitude;
		} else {
			triangleLookupTable[i] = waveAmplitude * (i - threeQuartersOfTheWavePeriod) / quarterOfTheWavePeriod - waveAmplitude;
		}
	}
}

void feedSawtoothTable(int16_t* sawtoothLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	for (uint16_t i = 0; i < tableSize; i++) {
		sawtoothLookupTable[i] = (2 * waveAmplitude * i) / tableSize - waveAmplitude;
	}
}

void feedSquareTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	const uint16_t halfOfTheWave = tableSize >> 1;
	for (uint16_t i = 0; i < tableSize; i++) {
		if (i < halfOfTheWave) {
			squareLookupTable[i] = waveAmplitude;
		} else {
			squareLookupTable[i] = -waveAmplitude;
		}
	}
}

void feedDMAAudioBuffer(Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames){
	float output;
	const float antipopFactor = 0.001f;
	const float volumeSmoothing = 0.005f;
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
