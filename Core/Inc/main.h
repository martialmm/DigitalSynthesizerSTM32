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
#define SPI1_CLK_Display_Pin GPIO_PIN_5
#define SPI1_CLK_Display_GPIO_Port GPIOA
#define SPI1_MOSI_Display_Pin GPIO_PIN_7
#define SPI1_MOSI_Display_GPIO_Port GPIOA
#define MUX_POT_3_Pin GPIO_PIN_5
#define MUX_POT_3_GPIO_Port GPIOC
#define MUX_POT_2_Pin GPIO_PIN_0
#define MUX_POT_2_GPIO_Port GPIOB
#define MUX_POT_1_Pin GPIO_PIN_1
#define MUX_POT_1_GPIO_Port GPIOB
#define S0_MUX_Pin GPIO_PIN_2
#define S0_MUX_GPIO_Port GPIOB
#define S1_MUX_Pin GPIO_PIN_7
#define S1_MUX_GPIO_Port GPIOE
#define S2_MUX_Pin GPIO_PIN_8
#define S2_MUX_GPIO_Port GPIOE
#define Enable_MUX_Pin GPIO_PIN_9
#define Enable_MUX_GPIO_Port GPIOE
#define Delay_MonoStereo_Btn_Pin GPIO_PIN_8
#define Delay_MonoStereo_Btn_GPIO_Port GPIOD
#define Delay_OnOff_Btn_Pin GPIO_PIN_9
#define Delay_OnOff_Btn_GPIO_Port GPIOD
#define Reverb_OnOff_Btn_Pin GPIO_PIN_10
#define Reverb_OnOff_Btn_GPIO_Port GPIOD
#define Prev_Page_Display_Btn_Pin GPIO_PIN_13
#define Prev_Page_Display_Btn_GPIO_Port GPIOD
#define Next_Page_Display_Btn_Pin GPIO_PIN_14
#define Next_Page_Display_Btn_GPIO_Port GPIOD
#define bUpperOctave_Pin GPIO_PIN_9
#define bUpperOctave_GPIO_Port GPIOA
#define bLowerOctave_Pin GPIO_PIN_10
#define bLowerOctave_GPIO_Port GPIOA
#define bsquare_Pin GPIO_PIN_0
#define bsquare_GPIO_Port GPIOD
#define bsinus_Pin GPIO_PIN_1
#define bsinus_GPIO_Port GPIOD
#define btriangle_Pin GPIO_PIN_2
#define btriangle_GPIO_Port GPIOD
#define bsaw_Pin GPIO_PIN_3
#define bsaw_GPIO_Port GPIOD
#define Audio_RST_Pin GPIO_PIN_4
#define Audio_RST_GPIO_Port GPIOD
#define MUX_Waveforms_Pin GPIO_PIN_5
#define MUX_Waveforms_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
