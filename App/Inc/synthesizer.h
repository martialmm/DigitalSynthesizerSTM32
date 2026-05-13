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
void initSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator, UserInterface_t* userInterface);
void addOscillatorToSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator);
void synthesizerRun(Synthesizer_t* synthesizer, UserInterface_t* userInterface);
void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer);
void updateSynthesizerParameters(Synthesizer_t* synthesizer, UserInterface_t* userInterface);

#endif /* INC_SYNTHESIZER_H_ */
