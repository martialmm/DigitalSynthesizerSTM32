/*
 * user_interface.h
 *
 *  Created on: Mar 26, 2026
 *      Author: mars
 */

#ifndef INC_USER_INTERFACE_H_
#define INC_USER_INTERFACE_H_

#include <stdint.h>
#include "oscillator.h"

typedef struct LowPassFilter_EMA{
	float alpha; // [0..1]: 0 ==> max filtering / 1 ==> no filtering
	float output;
}LowPassFilter_EMA_t;

Waveform_t getUserWaveform(void);
float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband);
float approximateExpFunction(float linearScaledDeadbandPotentiometer);
float lowPassFilterPotentiometerInputs(float linearScaledDeadbandPotentiometer);
float processVolumePotentiometer(uint16_t potentiometerRawValue);
void startADCPotentiometer(ADC_HandleTypeDef *hadc);

extern uint32_t potentiometerRawValue;
extern volatile uint8_t conversionADCCompleted;

#endif /* INC_USER_INTERFACE_H_ */
