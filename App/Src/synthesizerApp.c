#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"
#include "oscillator.h"
#include "user_interface.h"

void synthesizer(){

	// Init phase
	initializeSynthesizer();
	initializeOscillator(&osc1);

	// I2S
	startI2SOscillator(i2sForExternalDAC);

	// TIMER
	HAL_TIM_Base_Start_IT(timerForUserInputsScan);

	while(1){
		scanPotentiometers();

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(&osc1, selectedWaveform);

	   // temp for tests
	   if(userInputs.buttonsState & BTN_LOWER_OCTAVE){
		osc1.frequency = 523.25f;
		osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, i2sForExternalDAC);
	   }

	   else if(userInputs.buttonsState & BTN_UPPER_OCTAVE){
		osc1.frequency = 783.99f;
		osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, i2sForExternalDAC);
	   }
	}
}
