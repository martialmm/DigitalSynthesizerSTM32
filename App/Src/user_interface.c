/*
 * user_interface.c
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#include "user_interface.h"
#include "main.h"

uint32_t potentiometerRawValue;
volatile uint8_t conversionADCCompleted = 0;
volatile UserInputs_t userInputs;

Waveform_t getUserWaveform(void)
{
//    if (HAL_GPIO_ReadPin(bsinus_GPIO_Port, bsinus_Pin))
//    {
//    	return SINUS;
//    }
//    else if (HAL_GPIO_ReadPin(btriangle_GPIO_Port, btriangle_Pin))
//    {
//    	return TRIANGLE;
//    }
//    else if (HAL_GPIO_ReadPin(bsaw_GPIO_Port, bsaw_Pin))
//    {
//    	return SAWTOOTH;
//    }
//    else if (HAL_GPIO_ReadPin(bsquare_GPIO_Port, bsquare_Pin))
//    {
//    	return SQUARE;
//    }
//    else
//    {
//    	return SINUS;
//    }
	return SINUS;
}

void scanUserInputs(){
	HAL_GPIO_WritePin(testLED_GPIO_Port, testLED_Pin, GPIO_PIN_RESET);
	if(HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bUpperOctave_Pin)){
		userInputs.buttonsState |= BTN_LOWER_OCTAVE;
	}
	if(HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)){
		userInputs.buttonsState |= BTN_UPPER_OCTAVE;
	}
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

