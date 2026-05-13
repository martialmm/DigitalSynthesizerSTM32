/*
 * user_interface.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "user_interface.h"
#include "main.h"

static float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband);
static float approximateExpFunction(float linearScaledDeadbandPotentiometer);
static float lowPassFilterPotentiometerInput(uint16_t linearScaledDeadbandPotentiometer);

static UserInterface_t* userInterface1 = NULL;


void initUserInterface(UserInterface_t* userInterface){
	userInterfaceRegister(userInterface);
	userInterface->currentMuxChannel = 0;
}

void userInterfaceRegister(UserInterface_t* userInterface) {
	userInterface1 = userInterface;
}


UserInterface_t* createUserInterface(void) {
    static UserInterface_t instance = {0};
    return &instance;
}


Waveform_t getUserWaveform(){
	if(userInterface1->userInputs.buttonsState & BTN_OSC1_SINUS) return SINUS;
	if(userInterface1->userInputs.buttonsState & BTN_OSC1_TRIANGLE) return TRIANGLE;
	if(userInterface1->userInputs.buttonsState & BTN_OSC1_SAWTOOTH) return SAWTOOTH;
	if(userInterface1->userInputs.buttonsState & BTN_OSC1_SQUARE) return SQUARE;
	return NONE;
}

void selectWaveformsMuxChannel(uint8_t channel){
	HAL_GPIO_WritePin(S0_All_MUX_GPIO_Port, S0_All_MUX_Pin, (channel & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S1_All_MUX_GPIO_Port, S1_All_MUX_Pin, (channel & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S2_All_MUX_GPIO_Port, S2_All_MUX_Pin, (channel & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void scanPushButtonsInputsForNotes() {
	// Temp Push buttons to trigger sounds
	if (HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin)) {
		userInterface1->userInputs.buttonsState |= BTN_LOWER_OCTAVE;
	} else {
		userInterface1->userInputs.buttonsState &= ~BTN_LOWER_OCTAVE;
	}
	if (HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)) {
		userInterface1->userInputs.buttonsState |= BTN_UPPER_OCTAVE;
	} else {
		userInterface1->userInputs.buttonsState &= ~BTN_UPPER_OCTAVE;
	}
}

void scanWaveformsSwitches() {
	// Multiplexer scan for waveforms switches
		GPIO_PinState pinState = HAL_GPIO_ReadPin( MUX_Switch_Waveforms_GPIO_Port, MUX_Switch_Waveforms_Pin);
		switch (userInterface1->currentMuxChannel) {
		case 0:
			if (pinState)
				userInterface1->userInputs.buttonsState |= BTN_OSC1_SINUS;
			else
				userInterface1->userInputs.buttonsState &= ~BTN_OSC1_SINUS;

			break;
		case 1:
			if (pinState)
				userInterface1->userInputs.buttonsState |= BTN_OSC1_TRIANGLE;
			else
				userInterface1->userInputs.buttonsState &= ~BTN_OSC1_TRIANGLE;

			break;
		case 2:
			if (pinState)
				userInterface1->userInputs.buttonsState |= BTN_OSC1_SAWTOOTH;
			else
				userInterface1->userInputs.buttonsState &= ~BTN_OSC1_SAWTOOTH;

			break;
		case 3:
			if (pinState)
				userInterface1->userInputs.buttonsState |= BTN_OSC1_SQUARE;
			else
				userInterface1->userInputs.buttonsState &= ~BTN_OSC1_SQUARE;

			break;
		default:
			break;
		}
}

float processVolumePotentiometer(uint16_t potentiometerRawValue){
	float filtered = lowPassFilterPotentiometerInput(potentiometerRawValue);

	float normalized = createDeadbandForPotentiometer(filtered, 25.0f);

	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume
	return approximateExpFunction(normalized);
}

static float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband) {
	float linearScaledDeadbandPotentiometer;

	if (potentiometerRawValue < potentiometerDeadband) {
		linearScaledDeadbandPotentiometer = 0.0f;
	} else {
		linearScaledDeadbandPotentiometer = (potentiometerRawValue - potentiometerDeadband) / (4095.0f - potentiometerDeadband);
	}
	return linearScaledDeadbandPotentiometer;
}

static float approximateExpFunction(float linearScaledDeadbandPotentiometer) {
	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume.
	return linearScaledDeadbandPotentiometer * linearScaledDeadbandPotentiometer;
}

static float lowPassFilterPotentiometerInput(uint16_t potentiometerValue) {
	// Filtering ADC inputs with Exponential Moving Average filter
	static LowPassFilter_EMA_t lowPassFilterEMA;

	// init low pass filter to get clean potentiometer ADC inputs
	lowPassFilterEMA.alpha = 0.1f;

	lowPassFilterEMA.output = lowPassFilterEMA.alpha * potentiometerValue + (1 - lowPassFilterEMA.alpha) * lowPassFilterEMA.output;
	return lowPassFilterEMA.output;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	// To fill potentiometers values: userInputs.potentiometersRaw --> tab of size 24
	// Each potentiometer has its own index
	// I use 3 channels of the ADC because I have 3 mux
	userInterface1->userInputs.potentiometersRaw[0 * 8 + userInterface1->currentMuxChannel] = userInterface1->potentiometersADCConversionBuffer[0];	// fill 0..7 indexes
	userInterface1->userInputs.potentiometersRaw[1 * 8 + userInterface1->currentMuxChannel] = userInterface1->potentiometersADCConversionBuffer[1]; // fill 8..15 indexes
	userInterface1->userInputs.potentiometersRaw[2 * 8 + userInterface1->currentMuxChannel] = userInterface1->potentiometersADCConversionBuffer[2]; // fill 16..23 indexes

	userInterface1->currentMuxChannel++;
	if(userInterface1->currentMuxChannel >= 8) userInterface1->currentMuxChannel = 0;
}
