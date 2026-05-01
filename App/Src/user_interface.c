/*
 * user_interface.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "user_interface.h"
#include "main.h"

static void scanPushButtonsInputsForNotes(void);
static void scanWaveformsSwitches(void);

uint32_t potentiometerRawValue;
volatile uint8_t conversionADCCompleted = 0;
volatile UserInputs_t userInputs;
TIM_HandleTypeDef* timerForUserInputsScan = NULL;

Waveform_t getUserWaveform(void){
	if(userInputs.buttonsState & BTN_OSC1_SINUS) return SINUS;
	if(userInputs.buttonsState & BTN_OSC1_TRIANGLE) return TRIANGLE;
	if(userInputs.buttonsState & BTN_OSC1_SAWTOOTH) return SAWTOOTH;
	if(userInputs.buttonsState & BTN_OSC1_SQUARE) return SQUARE;
	return NONE;
}

void selectWaveformsMuxChannel(uint8_t channel){
	HAL_GPIO_WritePin(S0_All_MUX_GPIO_Port, S0_All_MUX_Pin, (channel & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S1_All_MUX_GPIO_Port, S1_All_MUX_Pin, (channel & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S2_All_MUX_GPIO_Port, S2_All_MUX_Pin, (channel & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void scanPushButtonsInputsForNotes() {
	// Temp Push buttons to trigger sounds
	if (HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin)) {
		userInputs.buttonsState |= BTN_LOWER_OCTAVE;
	} else {
		userInputs.buttonsState &= ~BTN_LOWER_OCTAVE;
	}
	if (HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)) {
		userInputs.buttonsState |= BTN_UPPER_OCTAVE;
	} else {
		userInputs.buttonsState &= ~BTN_UPPER_OCTAVE;
	}
}

static void scanWaveformsSwitches() {
	// Multiplexer scan for waveforms switches
	for (int channel = 0; channel < 4; channel++) {
		selectWaveformsMuxChannel(channel);
		GPIO_PinState pinState = HAL_GPIO_ReadPin(
				MUX_Switch_Waveforms_GPIO_Port, MUX_Switch_Waveforms_Pin);
		switch (channel) {
		case 0:
			if (pinState)
				userInputs.buttonsState |= BTN_OSC1_SINUS;
			else
				userInputs.buttonsState &= ~BTN_OSC1_SINUS;

			break;
		case 1:
			if (pinState)
				userInputs.buttonsState |= BTN_OSC1_TRIANGLE;
			else
				userInputs.buttonsState &= ~BTN_OSC1_TRIANGLE;

			break;
		case 2:
			if (pinState)
				userInputs.buttonsState |= BTN_OSC1_SAWTOOTH;
			else
				userInputs.buttonsState &= ~BTN_OSC1_SAWTOOTH;

			break;
		case 3:
			if (pinState)
				userInputs.buttonsState |= BTN_OSC1_SQUARE;
			else
				userInputs.buttonsState &= ~BTN_OSC1_SQUARE;

			break;
		}
	}
}

void scanUserInputs(){
	scanPushButtonsInputsForNotes();
	scanWaveformsSwitches();
}

float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband) {
	float linearScaledDeadbandPotentiometer;

	if (potentiometerRawValue < potentiometerDeadband) {
		linearScaledDeadbandPotentiometer = 0.0f;
	} else {
		linearScaledDeadbandPotentiometer = (potentiometerRawValue - potentiometerDeadband) / (4095.0f - potentiometerDeadband);
	}
	return linearScaledDeadbandPotentiometer;
}

float approximateExpFunction(float linearScaledDeadbandPotentiometer) {
	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume.
	return linearScaledDeadbandPotentiometer * linearScaledDeadbandPotentiometer;
}

float lowPassFilterPotentiometerInput(uint16_t potentiometerValue) {
	// Filtering ADC inputs with Exponential Moving Average filter
	static LowPassFilter_EMA_t lowPassFilterEMA;
	float linearScaledDeadbandPotentiometer = 0.0f;
	const float potentiometerDeadband = 25.0f;

	// Potentiometer Deadband
	linearScaledDeadbandPotentiometer = createDeadbandForPotentiometer((float)potentiometerValue, potentiometerDeadband);

	// init low pass filter to get clean potentiometer ADC inputs
	lowPassFilterEMA.alpha = 0.1f;

	lowPassFilterEMA.output = lowPassFilterEMA.alpha * linearScaledDeadbandPotentiometer + (1 - lowPassFilterEMA.alpha) * lowPassFilterEMA.output;
	return lowPassFilterEMA.output;
}

float processVolumePotentiometer(uint16_t potentiometerRawValue){
	float expScaledPotentiometer = 0.0f;

	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume.
	expScaledPotentiometer = approximateExpFunction(expScaledPotentiometer);

	return lowPassFilterPotentiometerInput(expScaledPotentiometer);
}

void startADCPotentiometer(ADC_HandleTypeDef *hadc) {
    HAL_ADC_Start_DMA(hadc, &potentiometerRawValue, 1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	conversionADCCompleted = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == timerForUserInputsScan){
		scanUserInputs();
	}
}

