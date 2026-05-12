/*
 * synthesizer.c
 *
 *  Created on: May 12, 2026
 *      Author: mars
 */

#include "synthesizer.h"
#include "main.h"

ADC_HandleTypeDef* adcForPotentiometers = NULL;
TIM_HandleTypeDef* timerForUserInputsScan = NULL;
volatile uint8_t scanUserInputsFlag = 0;

Synthesizer_t* createSynthesizer() {
    static Synthesizer_t instance = {0};
    return &instance;
}

void initializeSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator){
	HAL_GPIO_WritePin(Enable_All_MUX_GPIO_Port, Enable_All_MUX_Pin, GPIO_PIN_RESET);
	initializeOscillator(oscillator);
	oscillatorRegister(oscillator);
	addOscillatorToSynthesizer(synthesizer, oscillator);
}

void addOscillatorToSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator){
	synthesizer->oscillator = oscillator;
}

void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer){
	setOscillatorVolume(synthesizer->oscillator, getOscillatorVolume(synthesizer->oscillator));
	setOscillatorWaveform(synthesizer->oscillator, getUserWaveform());
}

void updateSynthParameters(Synthesizer_t* synthesizer, UserInterface_t* userInterface){
	if(scanUserInputsFlag){
		selectWaveformsMuxChannel(userInterface->currentMuxChannel);
		scanPushButtonsInputsForNotes();
		scanWaveformsSwitches();
		HAL_ADC_Start_DMA(adcForPotentiometers, (uint32_t*)userInterface->potentiometersADCConversionBuffer, 3);

		// ADC mux master volume
		float targetVolume = processVolumePotentiometer(userInterface->userInputs.potentiometersRaw[POT_OSC1_VOL]);
		setOscillatorVolume(synthesizer->oscillator, targetVolume);

		scanUserInputsFlag = 0;
	}

	if((userInterface->userInputs.buttonsState & BTN_LOWER_OCTAVE) || (userInterface->userInputs.buttonsState & BTN_UPPER_OCTAVE)){
		noteIsPlayed(synthesizer->oscillator);
	}
	else{
		noteIsNotPlayed(synthesizer->oscillator);
	}

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	// Periodic timer interrupt for user inputs scan: switches (polling) + potentiometers (adc + dma)
	if(htim == timerForUserInputsScan){
		scanUserInputsFlag = 1;
	}
}
