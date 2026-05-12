/*
 * synth_params.h
 *
 *  Created on: May 11, 2026
 *      Author: mars
 */

#ifndef INC_SYNTHESIZER_H_
#define INC_SYNTHESIZER_H_
#include "oscillator.h"
#include "user_interface.h"


typedef struct {
	// Oscillators
	Oscillator_t* oscillator;
}Synthesizer_t;

Synthesizer_t* createSynthesizer(void);
void initializeSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator);
void addOscillatorToSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator);
void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer);
void updateSynthParameters(Synthesizer_t* synthesizer, UserInterface_t* userInterface);

#endif /* INC_SYNTHESIZER_H_ */
