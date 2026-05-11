#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"

void synthesizer(){

	Synthesizer_t synthesizer;

	// Init phase
	initializeSynthesizer();
	initializeOscillator(&oscillator1);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		updateSynthParameters(&synthesizer);
		updateSynthesizerOscillatorState(&oscillator1, &synthesizer);

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(&oscillator1, selectedWaveform);

	   // temp for tests
	   if(userInputs.buttonsState & BTN_LOWER_OCTAVE){
		oscillator1.frequency = 523.25f;
		oscillator1.phaseIncrement = computePhaseIncrement(oscillator1.frequency, i2sForExternalDAC);
	   }

	   else if(userInputs.buttonsState & BTN_UPPER_OCTAVE){
		oscillator1.frequency = 783.99f;
		oscillator1.phaseIncrement = computePhaseIncrement(oscillator1.frequency, i2sForExternalDAC);
	   }
	}
}
