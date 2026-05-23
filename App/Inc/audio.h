/*
 * audio.h
 *
 *  Created on: May 15, 2026
 *      Author: mars
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include "oscillator.h"
#include "envelope.h"

#define NUMBER_OF_FRAMES_PER_HALF 32  // 32 samples (left+right) for each call
#define TOTAL_BUFFER_SIZE (NUMBER_OF_FRAMES_PER_HALF * 2 * 2) // 32 frames * 2 (L/R) * 2 (halves) = 128 values

void startI2SOscillator(I2S_HandleTypeDef* hi2s);
void oscillatorRegister(Oscillator_t* oscillatorToAdd);
void audioEnvelopeRegister(Envelope_t* envelopeToAdd);

#endif /* INC_AUDIO_H_ */
