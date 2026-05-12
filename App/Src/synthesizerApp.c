#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"

void synthesizer(){

	Synthesizer_t synthesizer;
	Oscillator_t* oscillator1 = createOscillator();

	// Init phase
	initializeSynthesizer();
	initializeOscillator(oscillator1);
	oscillatorRegister(oscillator1);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		updateSynthParameters(&synthesizer);
		updateSynthesizerOscillatorState(oscillator1, &synthesizer);

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(oscillator1, selectedWaveform);

	   // temp for tests
	   if(userInputs.buttonsState & BTN_LOWER_OCTAVE){
		   setOscillatorFrequency(oscillator1, 523.25f);
		   setOscillatorPhaseIncrement(oscillator1, computePhaseIncrement(getOscillatorFrequency(oscillator1), i2sForExternalDAC));
	   }

	   else if(userInputs.buttonsState & BTN_UPPER_OCTAVE){
	   setOscillatorFrequency(oscillator1, 783.99f);
	   setOscillatorPhaseIncrement(oscillator1, computePhaseIncrement(getOscillatorFrequency(oscillator1), i2sForExternalDAC));
	   }
	}
}
