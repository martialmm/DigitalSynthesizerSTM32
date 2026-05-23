/*
 * audio.h
 *
 *  Created on: May 15, 2026
 *      Author: mars
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include "oscillator.h"

void startI2SOscillator(I2S_HandleTypeDef* hi2s);
void oscillatorRegister(Oscillator_t* oscillatorToAdd);

#endif /* INC_AUDIO_H_ */
