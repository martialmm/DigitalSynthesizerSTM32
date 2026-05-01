/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define S0_All_MUX_Pin GPIO_PIN_0
#define S0_All_MUX_GPIO_Port GPIOA
#define S1_All_MUX_Pin GPIO_PIN_1
#define S1_All_MUX_GPIO_Port GPIOA
#define S2_All_MUX_Pin GPIO_PIN_2
#define S2_All_MUX_GPIO_Port GPIOA
#define Enable_All_MUX_Pin GPIO_PIN_5
#define Enable_All_MUX_GPIO_Port GPIOA
#define MUX_POT_FX_AND_MSTRVOL_Pin GPIO_PIN_7
#define MUX_POT_FX_AND_MSTRVOL_GPIO_Port GPIOA
#define MUX_POT_FILTER_AND_ADSR_Pin GPIO_PIN_0
#define MUX_POT_FILTER_AND_ADSR_GPIO_Port GPIOB
#define MUX_POT_MIXER_Pin GPIO_PIN_1
#define MUX_POT_MIXER_GPIO_Port GPIOB
#define MUX_Switch_Waveforms_Pin GPIO_PIN_2
#define MUX_Switch_Waveforms_GPIO_Port GPIOB
#define I2S_CK_Ext_DAC_Pin GPIO_PIN_10
#define I2S_CK_Ext_DAC_GPIO_Port GPIOB
#define I2S_WS_Ext_DAC_Pin GPIO_PIN_12
#define I2S_WS_Ext_DAC_GPIO_Port GPIOB
#define I2S_SD_Ext_DAC_Pin GPIO_PIN_15
#define I2S_SD_Ext_DAC_GPIO_Port GPIOB
#define Delay_MonoStereo_Btn_Pin GPIO_PIN_8
#define Delay_MonoStereo_Btn_GPIO_Port GPIOA
#define Delay_OnOff_Btn_Pin GPIO_PIN_9
#define Delay_OnOff_Btn_GPIO_Port GPIOA
#define Reverb_OnOff_Btn_Pin GPIO_PIN_10
#define Reverb_OnOff_Btn_GPIO_Port GPIOA
#define bUpperOctave_Pin GPIO_PIN_11
#define bUpperOctave_GPIO_Port GPIOA
#define bLowerOctave_Pin GPIO_PIN_12
#define bLowerOctave_GPIO_Port GPIOA
#define debugLED_Pin GPIO_PIN_5
#define debugLED_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
