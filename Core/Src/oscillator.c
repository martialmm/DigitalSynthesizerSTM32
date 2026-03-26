/*
 * oscillator.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "oscillator.h"
#include <math.h>

#define WAVE_AMPLITUDE 16000
#define PIPI 6.2831853

static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
static int16_t sineLookupTable[SAMPLE_NUMBER_LUT];
static int16_t triangleLookupTable[SAMPLE_NUMBER_LUT];
static int16_t sawtoothLookupTable[SAMPLE_NUMBER_LUT];
static int16_t squareLookupTable[SAMPLE_NUMBER_LUT];
struct Oscillator_t osc1;

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&osc1, &dmaAudioBuffer[TOTAL_BUFFER_SIZE / 2], NUMBER_OF_FRAMES_PER_HALF);
}

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0); // 4294967296.0 = 2^32
}

int16_t* defineActiveLookupTableWaveform(enum Waveform_t selectedWaveform){
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
	feedSinewaveTable(sineLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
	feedTriangleTable(triangleLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
	feedSawtoothTable(sawtoothLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
	feedSquareTable(squareLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
}

void initializeSynthesizer(){
	createAllLookupTables();
}

void initializeOscillator(struct Oscillator_t* oscillator){
	oscillator->activeLookupTable = sineLookupTable;
	oscillator->detune = 0;
	oscillator->enveloppe = 0.0f;
	oscillator->frequency = 0.0f;
	oscillator->phase = 0;
	oscillator->phaseIncrement = 0;
	oscillator->volume = 0.0f;
	oscillator->waveform = SINUS;
}

void setOscillatorWaveform(struct Oscillator_t *osc, enum Waveform_t waveform) {
    if (waveform != NONE && waveform != osc->waveform) {
        osc->waveform = waveform;
        osc->activeLookupTable = defineActiveLookupTableWaveform(waveform);
    }
}

void feedSinewaveTable(int16_t* sinusLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	for (uint16_t i = 0; i < tableSize; i++) {
		sinusLookupTable[i] = (int16_t) (waveAmplitude * sin(i * PIPI / tableSize));
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

void feedDMAAudioBuffer(struct Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames){
	float output;
	const float antipopFactor = 0.001;
	uint8_t noteButtonPressed = HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin) || HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin);

	for(uint16_t i = 0; i < num_frames; i++){
		if(noteButtonPressed){
			oscillator->enveloppe += antipopFactor;
			if(oscillator->enveloppe > 1.0) oscillator->enveloppe = 1.0;
		}
		else{
			oscillator->enveloppe -= antipopFactor;
			if (oscillator->enveloppe < 0.0) oscillator->enveloppe = 0.0;
		}

		output = oscillator->activeLookupTable[oscillator->phase >> FP_SHIFT_AMOUNT] * oscillator->enveloppe * oscillator->volume;

		// securite pour pas perde un ou deux tympans
		if (output > 32767.0f) output = 32767.0f;
		if (output < -32768.0f) output = -32768.0f;

		buffer[2*i] = output;
		buffer[2*i+1] = output;

		oscillator->phase += oscillator->phaseIncrement;
	}
}
