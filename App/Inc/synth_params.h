/*
 * synth_params.h
 *
 *  Created on: May 11, 2026
 *      Author: mars
 */

#ifndef INC_SYNTH_PARAMS_H_
#define INC_SYNTH_PARAMS_H_
#include "synth_definitions.h"

typedef struct {
	// Oscillators
	Waveform_t osc1Waveform;
	float osc1Volume; 		// range: 0.0f 1.0f
	float osc1Frequency;
}Synthesizer_t;

#endif /* INC_SYNTH_PARAMS_H_ */
