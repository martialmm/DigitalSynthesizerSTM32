/*
 * oscillator.h
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_

#include <stdint.h>
#include "main.h"

#define NUMBER_OF_FRAMES_PER_HALF 32  // 32 samples (left+right) for each call
#define TOTAL_BUFFER_SIZE (NUMBER_OF_FRAMES_PER_HALF * 2 * 2) // 32 frames * 2 (L/R) * 2 (halves) = 128 values
#define LUT_BITS 12
#define SAMPLE_NUMBER_LUT (1 << LUT_BITS) // can hear a small harmonic distortion for value < 4096 => maybe something to improve
#define FP_SHIFT_AMOUNT (32 - LUT_BITS)

enum Waveform_t{
    NONE,
    SINUS,
    TRIANGLE,
    SAWTOOTH,
    SQUARE
};

struct Oscillator_t{
    float enveloppe;
    float frequency;
    float volume;
    uint32_t phase;
    uint32_t phaseIncrement;
    int16_t* activeLookupTable;
    int8_t detune;
    enum Waveform_t waveform;
};

float processVolumePotentiometer(uint16_t potentiometerRawValue);
uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s);
int16_t* defineActiveLookupTableWaveform(enum Waveform_t selectedWaveform);
void startI2SOscillator(I2S_HandleTypeDef* hi2s);
void createAllLookupTables();
void initializeSynthesizer();
void initializeOscillator(struct Oscillator_t* oscillator);
void setOscillatorWaveform(struct Oscillator_t *osc, enum Waveform_t waveform);
void feedSinewaveTable(int16_t* sinusLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedTriangleTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedSawtoothTable(int16_t* sawtoothLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedSquareTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude);
void feedDMAAudioBuffer(struct Oscillator_t* oscillator, int16_t* buffer, uint16_t num_frames);

extern struct Oscillator_t osc1;

#endif /* INC_OSCILLATOR_H_ */
