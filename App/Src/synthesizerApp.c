#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"
#include "oscillator.h"
#include "user_interface.h"
#include "synthesizer.h"

void synthesizer(){

	Synthesizer_t* synthesizer = createSynthesizer();
	Oscillator_t* oscillator1 = createOscillator();
	UserInterface_t* userInterface = createUserInterface();


	// Init phase
	initializeSynthesizer();

	initializeOscillator(oscillator1);
	oscillatorRegister(oscillator1);
	addOscillatorToSynthesizer(synthesizer, oscillator1);

	initUserInterface(userInterface);
	userInterfaceRegister(userInterface);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		updateSynthParameters(synthesizer, userInterface);
		updateSynthesizerOscillatorsState(synthesizer);

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(oscillator1, selectedWaveform);

	   // temp for tests
	   if(userInterface->userInputs.buttonsState & BTN_LOWER_OCTAVE){
		   setOscillatorFrequency(oscillator1, 523.25f);
		   setOscillatorPhaseIncrement(oscillator1, computePhaseIncrement(getOscillatorFrequency(oscillator1), i2sForExternalDAC));
	   }

	   else if(userInterface->userInputs.buttonsState & BTN_UPPER_OCTAVE){
	   setOscillatorFrequency(oscillator1, 783.99f);
	   setOscillatorPhaseIncrement(oscillator1, computePhaseIncrement(getOscillatorFrequency(oscillator1), i2sForExternalDAC));
	   }
	}
}
