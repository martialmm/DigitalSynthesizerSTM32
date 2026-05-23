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
#include "synth_definitions.h"

#define NUMBER_OF_FRAMES_PER_HALF 32  // 32 samples (left+right) for each call
#define TOTAL_BUFFER_SIZE (NUMBER_OF_FRAMES_PER_HALF * 2 * 2) // 32 frames * 2 (L/R) * 2 (halves) = 128 values
#define LUT_BITS 12
#define FP_SHIFT_AMOUNT (32 - LUT_BITS)

typedef struct Oscillator Oscillator_t;

Oscillator_t* createOscillator(void);
uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s);
void initOscillator(Oscillator_t* oscillator);
float getNextOscillatorSample(Oscillator_t* oscillator);

// SETTERS
void setOscillatorWaveform(Oscillator_t *oscillator, Waveform_t waveform);
void setOscillatorFrequency(Oscillator_t* oscillator, float frequency);
void setOscillatorPhaseIncrement(Oscillator_t* oscillator, uint32_t phaseIncrement);
void setOscillatorTargetVolume(Oscillator_t* oscillator, float targetVolume);
void setOscillatorCurrentvolume(Oscillator_t* oscillator, float currentVolume);
void noteIsPlayed(Oscillator_t* oscillator);
void noteIsNotPlayed(Oscillator_t* oscillator);

// GETTERS
float getOscillatorFrequency(Oscillator_t* oscillator);
float getOscillatorTargetVolume(Oscillator_t* oscillator);
float getOscillatorCurrentVolume(Oscillator_t* oscillator);


#endif /* INC_OSCILLATOR_H_ */
