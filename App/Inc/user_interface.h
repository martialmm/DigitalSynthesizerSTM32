/*
 * user_interface.h
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#ifndef INC_USER_INTERFACE_H_
#define INC_USER_INTERFACE_H_

#include <stdint.h>
#include "oscillator.h"


// SWITCHES
#define BTN_OSC1_SINUS		 (1 << 0)
#define BTN_OSC1_TRIANGLE	 (1 << 1)
#define BTN_OSC1_SAWTOOTH	 (1 << 2)
#define BTN_OSC1_SQUARE		 (1 << 3)
#define BTN_OSC2_SINUS 		 (1 << 4)
#define BTN_OSC2_TRIANGLE	 (1 << 5)
#define BTN_OSC2_SAWTOOTH	 (1 << 6)
#define BTN_OSC2_SQUARE		 (1 << 7)
#define BTN_REVERB_TOGGLE	 (1 << 8)
#define BTN_DELAY_TOGGLE	 (1 << 9)
#define BTN_DELAY_MONOSTEREO (1 << 10)
#define BTN_LOWER_OCTAVE 	 (1 << 11) // for debugging only
#define BTN_UPPER_OCTAVE	 (1 << 12) // for debugging only


// POTENTIOMETERS
#define NUMBER_OF_POTS			24
#define POT_OSC1_VOL			0
#define POT_OSC1_OCTAVE 		1
#define POT_OSC2_VOL			2
#define POT_OSC2_OCTAVE 		3
#define POT_OSC2_DETUNE 		4
#define POT_NOISE				5

#define POT_FILTER_CUTOFF 		8
#define POT_FILTER_RESONANCE	9
#define POT_FILTER_ENVELOPPPE	10
#define POT_ENV_ATTACK			11
#define POT_ENV_DECAY			12
#define POT_ENV_SUSTAIN			13
#define POT_ENV_RELEASE			14

#define POT_REVERB_HIGHPASS 	16
#define POT_REVERB_DECAY 		17
#define POT_REVERB_DRYWET		18
#define POT_DELAY_RATE			19
#define POT_DELAY_DRYWET		20
#define POT_MASTER_VOLUME		21


typedef struct {
	float alpha; // [0..1]: 0 ==> max filtering / 1 ==> no filtering
	float output;
} LowPassFilter_EMA_t;

typedef struct {
	uint32_t buttonsState;
	uint32_t potentiometersRaw[NUMBER_OF_POTS];
	uint32_t potentiometersFiltered[NUMBER_OF_POTS];
} UserInputs_t;

Waveform_t getUserWaveform(void);
float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband);
float approximateExpFunction(float linearScaledDeadbandPotentiometer);
float lowPassFilterPotentiometerInput(uint16_t linearScaledDeadbandPotentiometer);
float processVolumePotentiometer(uint16_t potentiometerRawValue);
void selectWaveformsMuxChannel(uint8_t channel);
void scanUserInputs();
void scanPotentiometers();

extern UserInputs_t userInputs;

#endif /* INC_USER_INTERFACE_H_ */
