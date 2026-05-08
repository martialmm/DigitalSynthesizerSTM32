/*
 * oscillator.h
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define NUMBER_OF_FRAMES_PER_HALF 32  // 32 samples (left+right) for each call
#define TOTAL_BUFFER_SIZE (NUMBER_OF_FRAMES_PER_HALF * 2 * 2) // 32 frames * 2 (L/R) * 2 (halves) = 128 values
#define LUT_BITS 12
#define SAMPLE_NUMBER_LUT (1 << LUT_BITS) // can hear a small harmonic distortion for value < 4096 => maybe something to improve
#define FP_SHIFT_AMOUNT (32 - LUT_BITS)

typedef enum{
    NONE,
    SINUS,
    TRIANGLE,
    SAWTOOTH,
    SQUARE
}Waveform_t;

typedef struct {
    float enveloppe;
    float frequency;
    float volume;
    uint32_t phase;
    uint32_t phaseIncrement;
    const int16_t* activeLookupTable;
    int8_t detune;
    Waveform_t waveform;
}Oscillator_t;

float processVolumePotentiometer(uint16_t potentiometerRawValue);
uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s);
const int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform);
void startI2SOscillator(I2S_HandleTypeDef* hi2s);
void createAllLookupTables();
void initializeSynthesizer();
void initializeOscillator(Oscillator_t* oscillator);
void setOscillatorWaveform(Oscillator_t *osc,Waveform_t waveform);
void feedTriangleTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedSawtoothTable(int16_t* sawtoothLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedSquareTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedDMAAudioBuffer(Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames);

extern Oscillator_t osc1;

#endif /* INC_OSCILLATOR_H_ */
