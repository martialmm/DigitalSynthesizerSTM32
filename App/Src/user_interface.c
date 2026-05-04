/*
 * user_interface.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "user_interface.h"
#include "main.h"

static void scanPushButtonsInputsForNotes(void);
static void scanWaveformsSwitches(uint8_t current_mux_channel);

UserInputs_t userInputs;
TIM_HandleTypeDef* timerForUserInputsScan = NULL;
ADC_HandleTypeDef* adcForPotentiometers = NULL;
uint16_t potentiometersADCConversionBuffer[3];
volatile uint8_t currentMuxChannel = 0;


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

static void scanWaveformsSwitches(uint8_t current_mux_channel) {
	// Multiplexer scan for waveforms switches
		GPIO_PinState pinState = HAL_GPIO_ReadPin( MUX_Switch_Waveforms_GPIO_Port, MUX_Switch_Waveforms_Pin);
		switch (current_mux_channel) {
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
		default:
			break;
		}
}

void scanPotentiometers(){
	// ADC mux master volume
	osc1.volume = processVolumePotentiometer(userInputs.potentiometersRaw[POT_MASTER_VOLUME]);
}

void scanUserInputs(uint8_t current_mux_channel){
	scanPushButtonsInputsForNotes();
	scanWaveformsSwitches(current_mux_channel);
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

	// init low pass filter to get clean potentiometer ADC inputs
	lowPassFilterEMA.alpha = 0.1f;

	lowPassFilterEMA.output = lowPassFilterEMA.alpha * potentiometerValue + (1 - lowPassFilterEMA.alpha) * lowPassFilterEMA.output;
	return lowPassFilterEMA.output;
}

float processVolumePotentiometer(uint16_t potentiometerRawValue){
	float filtered = lowPassFilterPotentiometerInput(potentiometerRawValue);

	float normalized = createDeadbandForPotentiometer(filtered, 25.0f);

	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume
	return approximateExpFunction(normalized);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	// To fill potentiometers values: userInputs.potentiometersRaw --> tab of size 24
	// Each potentiometer has its own index
	// I use 3 channels of the ADC because I have 3 mux
	userInputs.potentiometersRaw[0 * 8 + currentMuxChannel] = potentiometersADCConversionBuffer[0];	// fill 0..7 indexes
	userInputs.potentiometersRaw[1 * 8 + currentMuxChannel] = potentiometersADCConversionBuffer[1]; // fill 8..15 indexes
	userInputs.potentiometersRaw[2 * 8 + currentMuxChannel] = potentiometersADCConversionBuffer[2]; // fill 16..23 indexes

	currentMuxChannel++;
	if(currentMuxChannel >= 8) currentMuxChannel = 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	// Periodic timer interrupt for user inputs scan: switches (polling) + potentiometers (adc + dma)
	if(htim == timerForUserInputsScan){
		selectWaveformsMuxChannel(currentMuxChannel);
		scanUserInputs(currentMuxChannel);
		HAL_ADC_Start_DMA(adcForPotentiometers, (uint32_t*)potentiometersADCConversionBuffer, 3);
	}
}
