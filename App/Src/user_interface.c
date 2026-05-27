/*
 * user_interface.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "user_interface.h"
#include "main.h"

static float createDeadbandForPotentiometer(uint32_t potentiometerRawValue, const float potentiometerDeadband);
static float approximateExpFunction(float linearScaledDeadbandPotentiometer);
static float lowPassFilterPotentiometerInput(UserInterface_t* userInterface, uint8_t potentiometer);

typedef struct {
	float alpha; // [0..1]: 0 ==> max filtering / 1 ==> no filtering
	float output;
} LowPassFilter_EMA_t;

typedef struct {
	uint32_t buttonsState;
	uint32_t potentiometersRaw[NUMBER_OF_POTS];
	uint32_t potentiometersFiltered[NUMBER_OF_POTS];
} UserInputs_t;

struct UserInterface{
	UserInputs_t userInputs;
	LowPassFilter_EMA_t potentiometerFilterEMA[NUMBER_OF_POTS];
	uint16_t potentiometersADCConversionBuffer[3];
	volatile uint8_t currentMuxChannelSelected;
};

static UserInterface_t* userInterface1 = NULL;


// ---- INITIALIZATION ---- //

void initUserInterface(UserInterface_t* userInterface){
	userInterfaceRegister(userInterface);
	userInterface->currentMuxChannelSelected = 0;
}

void userInterfaceRegister(UserInterface_t* userInterface) {
	userInterface1 = userInterface;
}


UserInterface_t* createUserInterface(void) {
    static UserInterface_t instance = {0};
    return &instance;
}


// ---- GETTER ---- //

volatile uint8_t getCurrentMuxChannelSelected(UserInterface_t* userInterface){
	return userInterface->currentMuxChannelSelected;
}

uint16_t* getPotentiometersADCConversionBuffer(UserInterface_t* userInterface){
	return userInterface->potentiometersADCConversionBuffer;
}

uint32_t getPotentiometerRaw(UserInterface_t* userInterface, uint8_t potentiometerToRead){
	return userInterface->userInputs.potentiometersRaw[potentiometerToRead];
}

uint32_t getButtonsState(UserInterface_t* userInterface){
	return userInterface->userInputs.buttonsState;
}

// ---- PUBLIC FUNCTIONS ---- //

Waveform_t getUserWaveform(UserInterface_t* userInterface){
	if(userInterface->userInputs.buttonsState & BTN_OSC1_SINUS) return SINUS;
	if(userInterface->userInputs.buttonsState & BTN_OSC1_TRIANGLE) return TRIANGLE;
	if(userInterface->userInputs.buttonsState & BTN_OSC1_SAWTOOTH) return SAWTOOTH;
	if(userInterface->userInputs.buttonsState & BTN_OSC1_SQUARE) return SQUARE;
	return NONE;
}

void selectWaveformsMuxChannel(uint8_t channel){
	HAL_GPIO_WritePin(S0_All_MUX_GPIO_Port, S0_All_MUX_Pin, (channel & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S1_All_MUX_GPIO_Port, S1_All_MUX_Pin, (channel & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(S2_All_MUX_GPIO_Port, S2_All_MUX_Pin, (channel & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void scanPushButtonsInputsForNotes(UserInterface_t* userInterface) {
	// Temp Push buttons to trigger sounds
	if (HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin)) {
		userInterface->userInputs.buttonsState |= BTN_LOWER_OCTAVE;
	} else {
		userInterface->userInputs.buttonsState &= ~BTN_LOWER_OCTAVE;
	}
	if (HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)) {
		userInterface->userInputs.buttonsState |= BTN_UPPER_OCTAVE;
	} else {
		userInterface->userInputs.buttonsState &= ~BTN_UPPER_OCTAVE;
	}
}

void scanWaveformsSwitches(UserInterface_t* userInterface) {
	// Multiplexer scan for waveforms switches
		GPIO_PinState pinState = HAL_GPIO_ReadPin( MUX_Switch_Waveforms_GPIO_Port, MUX_Switch_Waveforms_Pin);
		switch (userInterface->currentMuxChannelSelected) {
		case 0:
			if (pinState)
				userInterface->userInputs.buttonsState |= BTN_OSC1_SINUS;
			else
				userInterface->userInputs.buttonsState &= ~BTN_OSC1_SINUS;

			break;
		case 1:
			if (pinState)
				userInterface->userInputs.buttonsState |= BTN_OSC1_TRIANGLE;
			else
				userInterface->userInputs.buttonsState &= ~BTN_OSC1_TRIANGLE;

			break;
		case 2:
			if (pinState)
				userInterface->userInputs.buttonsState |= BTN_OSC1_SAWTOOTH;
			else
				userInterface->userInputs.buttonsState &= ~BTN_OSC1_SAWTOOTH;

			break;
		case 3:
			if (pinState)
				userInterface->userInputs.buttonsState |= BTN_OSC1_SQUARE;
			else
				userInterface->userInputs.buttonsState &= ~BTN_OSC1_SQUARE;

			break;
		default:
			break;
		}
}

float processAudioPotentiometer(UserInterface_t* userInterface, uint8_t potentiometer){
	uint32_t filteredValue = lowPassFilterPotentiometerInput(userInterface, potentiometer);
	userInterface->userInputs.potentiometersFiltered[potentiometer] = filteredValue;

	float deadbandedPotentiometer = createDeadbandForPotentiometer(filteredValue, 25.0f);

	return approximateExpFunction(deadbandedPotentiometer);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	// To fill potentiometers values: userInputs.potentiometersRaw --> tab of size 24
	// Each potentiometer has its own index
	// I use 3 channels of the ADC because I have 3 mux
	userInterface1->userInputs.potentiometersRaw[0 * 8 + userInterface1->currentMuxChannelSelected] = userInterface1->potentiometersADCConversionBuffer[0];	// fill 0..7 indexes
	userInterface1->userInputs.potentiometersRaw[1 * 8 + userInterface1->currentMuxChannelSelected] = userInterface1->potentiometersADCConversionBuffer[1]; // fill 8..15 indexes
	userInterface1->userInputs.potentiometersRaw[2 * 8 + userInterface1->currentMuxChannelSelected] = userInterface1->potentiometersADCConversionBuffer[2]; // fill 16..23 indexes

	userInterface1->currentMuxChannelSelected++;
	if(userInterface1->currentMuxChannelSelected >= 8) userInterface1->currentMuxChannelSelected = 0;
}


// ---- PRIVATE FUNCTIONS ---- //

static float createDeadbandForPotentiometer(uint32_t potentiometerRawValue, const float potentiometerDeadband) {
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

static float lowPassFilterPotentiometerInput(UserInterface_t* userInterface, uint8_t potentiometer) {
	// Filtering ADC inputs with Exponential Moving Average filter
	userInterface->potentiometerFilterEMA[potentiometer].alpha = 0.1f;
	userInterface->potentiometerFilterEMA[potentiometer].output =
			userInterface->potentiometerFilterEMA[potentiometer].alpha * userInterface->userInputs.potentiometersRaw[potentiometer] +
			(1 - userInterface->potentiometerFilterEMA[potentiometer].alpha) * userInterface->potentiometerFilterEMA[potentiometer].output;

    return userInterface->potentiometerFilterEMA[potentiometer].output;
}
