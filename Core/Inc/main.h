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
#define S0_MUX_Pin GPIO_PIN_0
#define S0_MUX_GPIO_Port GPIOA
#define S1_MUX_Pin GPIO_PIN_1
#define S1_MUX_GPIO_Port GPIOA
#define S2_MUX_Pin GPIO_PIN_2
#define S2_MUX_GPIO_Port GPIOA
#define Enable_MUX_Pin GPIO_PIN_5
#define Enable_MUX_GPIO_Port GPIOA
#define MUX_POT_3_Pin GPIO_PIN_7
#define MUX_POT_3_GPIO_Port GPIOA
#define MUX_POT_2_Pin GPIO_PIN_0
#define MUX_POT_2_GPIO_Port GPIOB
#define MUX_POT_1_Pin GPIO_PIN_1
#define MUX_POT_1_GPIO_Port GPIOB
#define MUX_Switch_Waveforms_Pin GPIO_PIN_2
#define MUX_Switch_Waveforms_GPIO_Port GPIOB
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
#define UART_TX_MIDI_Pin GPIO_PIN_15
#define UART_TX_MIDI_GPIO_Port GPIOA
#define SPI1_SCK_Display_Pin GPIO_PIN_3
#define SPI1_SCK_Display_GPIO_Port GPIOB
#define SPI1_MOSI_Display_Pin GPIO_PIN_5
#define SPI1_MOSI_Display_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define UART_RX_MIDI_Pin GPIO_PIN_7
#define UART_RX_MIDI_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
