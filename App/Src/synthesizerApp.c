#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"
#include "oscillator.h"
#include "user_interface.h"
#include "synthesizer.h"

void synthesizer(){

	Synthesizer_t* synthesizer = createSynthesizer();
	Oscillator_t* oscillator = createOscillator();
	UserInterface_t* userInterface = createUserInterface();
	Envelope_t* envelope = createEnvelope();

	// Init phase
	initSynthesizer(synthesizer, oscillator, userInterface, envelope);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		synthesizerRun(synthesizer);
	}
}
