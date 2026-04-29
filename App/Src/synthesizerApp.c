#include <string.h>
#include <stdio.h>
#include "synthesizerApp.h"
#include "oscillator.h"
#include "user_interface.h"
#include "main.h"

void synthesizer(){

	// Init phase
	initializeSynthesizer();
	initializeOscillator(&osc1);

	// I2S
	startI2SOscillator(ch_hi2s2);

	// ADC
	startADCPotentiometer(ch_hadc1);

	// TIMER
	HAL_TIM_Base_Start_IT(ch_htim10);

	while(1){
//	    if(conversionADCCompleted){
//	    	osc1.volume = processVolumePotentiometer(potentiometerRawValue);
//	    	conversionADCCompleted = 0;
//	    }

	    osc1.volume = 1.0f;

		Waveform_t selectedWaveform = getUserWaveform();
		setOscillatorWaveform(&osc1, selectedWaveform);

	   // temp for tests
	   if(HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin)){
		osc1.frequency = 523.25f;
		osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, ch_hi2s2);
	   }

	   else if(HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)){
		osc1.frequency = 783.99f;
		osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, ch_hi2s2);
	   }
	}
}
