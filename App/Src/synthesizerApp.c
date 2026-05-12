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

	// Init phase
	initializeSynthesizer(synthesizer, oscillator);
	initUserInterface(userInterface);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		updateSynthParameters(synthesizer, userInterface);
		updateSynthesizerOscillatorsState(synthesizer);

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(oscillator, selectedWaveform);

	   // temp for tests
	   if(userInterface->userInputs.buttonsState & BTN_LOWER_OCTAVE){
		   setOscillatorFrequency(oscillator, 523.25f);
		   setOscillatorPhaseIncrement(oscillator, computePhaseIncrement(getOscillatorFrequency(oscillator), i2sForExternalDAC));
	   }

	   else if(userInterface->userInputs.buttonsState & BTN_UPPER_OCTAVE){
	   setOscillatorFrequency(oscillator, 783.99f);
	   setOscillatorPhaseIncrement(oscillator, computePhaseIncrement(getOscillatorFrequency(oscillator), i2sForExternalDAC));
	   }
	}
}
