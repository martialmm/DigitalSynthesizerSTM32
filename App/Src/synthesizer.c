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
I2S_HandleTypeDef* i2sForExternalDAC = NULL;
volatile uint8_t scanUserInputsFlag = 0;

Synthesizer_t* createSynthesizer() {
    static Synthesizer_t instance = {0};
    return &instance;
}

void initSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator, UserInterface_t* userInterface){
	HAL_GPIO_WritePin(Enable_All_MUX_GPIO_Port, Enable_All_MUX_Pin, GPIO_PIN_RESET);
	initOscillator(oscillator);
	oscillatorRegister(oscillator);
	addOscillatorToSynthesizer(synthesizer, oscillator);
	initUserInterface(userInterface);
}

void addOscillatorToSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator){
	synthesizer->oscillator = oscillator;
}

void synthesizerRun(Synthesizer_t* synthesizer, UserInterface_t* userInterface){
	updateSynthesizerParameters(synthesizer, userInterface);
	updateSynthesizerOscillatorsState(synthesizer);
}

void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer){
	float oscillatorFrequency = getOscillatorFrequency(synthesizer->oscillator);
	setOscillatorVolume(synthesizer->oscillator, getOscillatorVolume(synthesizer->oscillator));
	setOscillatorWaveform(synthesizer->oscillator, getUserWaveform());
	setOscillatorFrequency(synthesizer->oscillator, oscillatorFrequency);
	setOscillatorPhaseIncrement(synthesizer->oscillator, computePhaseIncrement(oscillatorFrequency, i2sForExternalDAC));
}

void updateSynthesizerParameters(Synthesizer_t* synthesizer, UserInterface_t* userInterface){
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

	// temp for tests
	if(userInterface->userInputs.buttonsState & BTN_LOWER_OCTAVE){
		setOscillatorFrequency(synthesizer->oscillator, 523.25f);
		noteIsPlayed(synthesizer->oscillator);
	}
	else if(userInterface->userInputs.buttonsState & BTN_UPPER_OCTAVE){
		setOscillatorFrequency(synthesizer->oscillator, 783.99f);
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
