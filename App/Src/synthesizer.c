/*
 * synthesizer.c
 *
 *  Created on: May 12, 2026
 *      Author: mars
 */

#include "synthesizer.h"
#include "main.h"

static void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer);
static void updateSynthesizerParameters(Synthesizer_t* synthesizer);


ADC_HandleTypeDef* adcForPotentiometers = NULL;
TIM_HandleTypeDef* timerForUserInputsScan = NULL;
I2S_HandleTypeDef* i2sForExternalDAC = NULL;
volatile uint8_t scanUserInputsFlag = 0;


// ---- INITIALIZATION ---- //

Synthesizer_t* createSynthesizer() {
    static Synthesizer_t instance = {0};
    return &instance;
}

void initSynthesizer(Synthesizer_t* synthesizer, Oscillator_t* oscillator, UserInterface_t* userInterface){
	HAL_GPIO_WritePin(Enable_All_MUX_GPIO_Port, Enable_All_MUX_Pin, GPIO_PIN_RESET);

	initOscillator(oscillator);
	oscillatorRegister(oscillator);
	synthesizer->oscillator = oscillator;

	initUserInterface(userInterface);
	synthesizer->userInterface = userInterface;

}


// ---- PUBLIC FUNCTION ---- //

void synthesizerRun(Synthesizer_t* synthesizer){
	updateSynthesizerParameters(synthesizer);
	updateSynthesizerOscillatorsState(synthesizer);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	// Periodic timer interrupt for user inputs scan: switches (polling) + potentiometers (adc + dma)
	if(htim == timerForUserInputsScan){
		scanUserInputsFlag = 1;
	}
}


// ---- PRIVATE FUNCTION ---- //

static void updateSynthesizerOscillatorsState(Synthesizer_t* synthesizer){
	float oscillatorFrequency = getOscillatorFrequency(synthesizer->oscillator);
	setOscillatorVolume(synthesizer->oscillator, getOscillatorVolume(synthesizer->oscillator));
	setOscillatorWaveform(synthesizer->oscillator, getUserWaveform());
	setOscillatorFrequency(synthesizer->oscillator, oscillatorFrequency);
	setOscillatorPhaseIncrement(synthesizer->oscillator, computePhaseIncrement(oscillatorFrequency, i2sForExternalDAC));
}

static void updateSynthesizerParameters(Synthesizer_t* synthesizer){
	if(scanUserInputsFlag){
		selectWaveformsMuxChannel(getCurrentMuxChannelSelected(synthesizer->userInterface));
		scanPushButtonsInputsForNotes();
		scanWaveformsSwitches();
		HAL_ADC_Start_DMA(adcForPotentiometers, (uint32_t*)getPotentiometersADCConversionBuffer(synthesizer->userInterface), 3);

		// ADC mux master volume
		float targetVolume = processVolumePotentiometer(getPotentiometerRaw(synthesizer->userInterface, POT_OSC1_VOL));
		setOscillatorVolume(synthesizer->oscillator, targetVolume);

		scanUserInputsFlag = 0;
	}

	// temp for tests
	if(getButtonsState(synthesizer->userInterface) & BTN_LOWER_OCTAVE){
		setOscillatorFrequency(synthesizer->oscillator, 523.25f);
		noteIsPlayed(synthesizer->oscillator);
	}
	else if(getButtonsState(synthesizer->userInterface) & BTN_UPPER_OCTAVE){
		setOscillatorFrequency(synthesizer->oscillator, 783.99f);
		noteIsPlayed(synthesizer->oscillator);
	}
	else{
		noteIsNotPlayed(synthesizer->oscillator);
	}
}
