/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "MY_CS43L22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUMBER_OF_FRAMES_PER_HALF 32  // 32 samples (left+right) for each call
#define TOTAL_BUFFER_SIZE (NUMBER_OF_FRAMES_PER_HALF * 2 * 2) // 32 frames * 2 (L/R) * 2 (halves) = 128 values
#define WAVE_AMPLITUDE 16000
#define PIPI 6.2831853
#define LUT_BITS 12
#define SAMPLE_NUMBER_LUT (1 << LUT_BITS) // can hear a small harmonic distortion for value < 4096 => maybe something to improve
#define FP_SHIFT_AMOUNT (32 - LUT_BITS)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
static int16_t dmaAudioBuffer[TOTAL_BUFFER_SIZE]; // double buffering --> we modify one half while the other half is being processed by the DMA (= automatically enable circucal mode)
static int16_t sineLookupTable[SAMPLE_NUMBER_LUT];
static int16_t triangleLookupTable[SAMPLE_NUMBER_LUT];
static int16_t sawtoothLookupTable[SAMPLE_NUMBER_LUT];
static int16_t squareLookupTable[SAMPLE_NUMBER_LUT];
Oscillator_t osc1;
volatile uint8_t conversionADCCompleted = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// used for debugging to print values on pc console with uart
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart4, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

Waveform_t getUserWaveform(void)
{
    if (HAL_GPIO_ReadPin(bsinus_GPIO_Port, bsinus_Pin))
    {
    	return SINUS;
    }
    else if (HAL_GPIO_ReadPin(btriangle_GPIO_Port, btriangle_Pin))
    {
    	return TRIANGLE;
    }
    else if (HAL_GPIO_ReadPin(bsaw_GPIO_Port, bsaw_Pin))
    {
    	return SAWTOOTH;
    }
    else if (HAL_GPIO_ReadPin(bsquare_GPIO_Port, bsquare_Pin))
    {
    	return SQUARE;
    }
    else
    {
    	return NONE;
    }
}

uint32_t computePhaseIncrement(float wantedWaveFrequency, I2S_HandleTypeDef *hi2s){
	return (uint32_t)(((double)wantedWaveFrequency / (double)hi2s->Init.AudioFreq) * 4294967296.0); // 4294967296.0 = 2^32
}

void feedDMAAudioBuffer(int16_t *buffer, uint16_t num_frames){
	float output;
	const float antipopFactor = 0.001;
	uint8_t noteButtonPressed = HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin) || HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin);

	for(uint16_t i = 0; i < num_frames; i++){
		if(noteButtonPressed){
			osc1.enveloppe += antipopFactor;
			if(osc1.enveloppe > 1.0) osc1.enveloppe = 1.0;
		}
		else{
			osc1.enveloppe -= antipopFactor;
			if (osc1.enveloppe < 0.0) osc1.enveloppe = 0.0;
		}

		output = osc1.activeLookupTable[osc1.phase >> FP_SHIFT_AMOUNT] * osc1.enveloppe * osc1.volume;

		// securite pour pas perde un ou deux tympans
		if (output > 32767.0f) output = 32767.0f;
		if (output < -32768.0f) output = -32768.0f;

		buffer[2*i] = output;
		buffer[2*i+1] = output;

		osc1.phase += osc1.phaseIncrement;
	}
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&dmaAudioBuffer[0], NUMBER_OF_FRAMES_PER_HALF);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
	feedDMAAudioBuffer(&dmaAudioBuffer[TOTAL_BUFFER_SIZE / 2], NUMBER_OF_FRAMES_PER_HALF);
}

void feedSinewaveTable(int16_t* sinusLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	for (uint16_t i = 0; i < tableSize; i++) {
		sinusLookupTable[i] = (int16_t) (waveAmplitude * sin(i * PIPI / tableSize));
	}
}

void feedTriangleTable(int16_t* triangleLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	//we slice one period of the triangle in three equations y = ax + b
	// y --> tab[i]
	// x --> i
	// b --> wave_amplitude
	// in each calculation, I have to do the multiplication first and then the division.
	// if I do the division first, the decimal part will be lost early (because we use integers) and the error will "snowball" with the multiplication.
	// This will lead to have bad values at the extremes points (or not really precise as we want).
	// Bit shifting is used to avoid some divisions.

	const uint16_t quarterOfTheWavePeriod = tableSize >> 2;
	const uint16_t halfOfTheWavePeriod = tableSize >> 1;
	const uint16_t threeQuartersOfTheWavePeriod = quarterOfTheWavePeriod + halfOfTheWavePeriod;

	for (uint16_t i = 0; i < tableSize; i++) {
		if (i < quarterOfTheWavePeriod) {
			triangleLookupTable[i] = (waveAmplitude * i) / quarterOfTheWavePeriod;
		} else if (i < threeQuartersOfTheWavePeriod) {
			triangleLookupTable[i] = - waveAmplitude * (i - quarterOfTheWavePeriod) / quarterOfTheWavePeriod + waveAmplitude;
		} else {
			triangleLookupTable[i] = waveAmplitude * (i - threeQuartersOfTheWavePeriod) / quarterOfTheWavePeriod - waveAmplitude;
		}
	}
}

void feedSawtoothTable(int16_t* sawtoothLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	for (uint16_t i = 0; i < tableSize; i++) {
		sawtoothLookupTable[i] = (2 * waveAmplitude * i) / tableSize - waveAmplitude;
	}
}

void feedSquareTable(int16_t* squareLookupTable, uint16_t tableSize, int32_t waveAmplitude) {
	const uint16_t halfOfTheWave = tableSize >> 1;
	for (uint16_t i = 0; i < tableSize; i++) {
		if (i < halfOfTheWave) {
			squareLookupTable[i] = waveAmplitude;
		} else {
			squareLookupTable[i] = -waveAmplitude;
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	conversionADCCompleted = 1;
}

float createDeadbandForPotentiometer(uint16_t potentiometerRawValue, const float potentiometerDeadband) {
	float linearScaledDeadbandPotentiometer;

	if (potentiometerRawValue < potentiometerDeadband) {
		linearScaledDeadbandPotentiometer = 0.0;
	} else {
		linearScaledDeadbandPotentiometer = (potentiometerRawValue - potentiometerDeadband) / (4095.0 - potentiometerDeadband);
	}
	return linearScaledDeadbandPotentiometer;
}

float approximateExpFunction(float linearScaledDeadbandPotentiometer) {
	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume.
	return linearScaledDeadbandPotentiometer * linearScaledDeadbandPotentiometer;
}

float lowPassFilterPotentiometerInputs(float linearScaledDeadbandPotentiometer) {
	// Filtering ADC inputs with Exponential Moving Average filter
	 LowPassFilter_EMA lowPassFilterEMA;
	// init low pass filter to get clean potentiometer ADC inputs
	lowPassFilterEMA.alpha = 0.1;
	lowPassFilterEMA.output = 0.0;

	lowPassFilterEMA.output = lowPassFilterEMA.alpha * linearScaledDeadbandPotentiometer + (1 - lowPassFilterEMA.alpha) * lowPassFilterEMA.output;
	return lowPassFilterEMA.output;
}

int16_t* defineActiveLookupTableWaveform(Waveform_t selectedWaveform){
	 if(selectedWaveform == SINUS){
		 return sineLookupTable;
	 }
	else if(selectedWaveform == TRIANGLE){
		return triangleLookupTable;
	}
	else if(selectedWaveform == SAWTOOTH){
		return sawtoothLookupTable;
	}
	else{
		return squareLookupTable;
	 }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  Waveform_t selectedWaveform;
  uint16_t potentiometerRawValue;
  // this variable is used to make sure we have a real "zeroed-volume" when potentiometer is at its physical zero value (which is never zero actually)
  // --> see "deadband"
  float linearScaledDeadbandPotentiometer;
  const float potentiometerDeadband = 25.0;
  const char* waveformsAvailable[] = {
      "NONE\r\n",
      "SINUS\r\n",
      "TRIANGLE\r\n",
      "SAW\r\n",
      "SQUARE\r\n"
  };
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  // Configure CS43 audio chip
  CS43_Init(hi2c1, MODE_I2S);
  CS43_SetVolume(1);
  CS43_Enable_RightLeft(CS43_RIGHT_LEFT);
  CS43_Start();

  // create lookup table for the 4 waveforms
  feedSinewaveTable(sineLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
  feedTriangleTable(triangleLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
  feedSawtoothTable(sawtoothLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);
  feedSquareTable(squareLookupTable, SAMPLE_NUMBER_LUT, WAVE_AMPLITUDE);

  // init oscillators
  osc1.activeLookupTable = sineLookupTable;
  osc1.detune = 0;
  osc1.enveloppe = 0.0f;
  osc1.frequency = 0.0f;
  osc1.phase = 0;
  osc1.phaseIncrement = 0;
  osc1.volume = 0.0f;
  osc1.waveform = SINUS;

  // i2s
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*) &dmaAudioBuffer, TOTAL_BUFFER_SIZE);

  // ADC in DMA mode
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &potentiometerRawValue, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1){
    if(conversionADCCompleted){

    	// Potentiometer Deadband
		linearScaledDeadbandPotentiometer = createDeadbandForPotentiometer(potentiometerRawValue, potentiometerDeadband);

    	// instead of having linear response, we approximate an exponential response (f(x) = x²) to have a more natural feeling when changing the volume.
		linearScaledDeadbandPotentiometer = approximateExpFunction(linearScaledDeadbandPotentiometer);

    	// Final output with Filtering ADC inputs with Exponential Moving Average filter
    	osc1.volume = lowPassFilterPotentiometerInputs(linearScaledDeadbandPotentiometer);

    	conversionADCCompleted = 0;
    }

    selectedWaveform = getUserWaveform();
    if (selectedWaveform != osc1.waveform && selectedWaveform != NONE){
    	osc1.waveform = selectedWaveform;
        HAL_UART_Transmit(&huart4, (uint8_t*) waveformsAvailable[osc1.waveform], strlen(waveformsAvailable[osc1.waveform]), 10);
        osc1.activeLookupTable = defineActiveLookupTableWaveform(selectedWaveform);
    }

   if(HAL_GPIO_ReadPin(bLowerOctave_GPIO_Port, bLowerOctave_Pin)){
	osc1.frequency = 523.25;
	osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, &hi2s3);
   }

   else if(HAL_GPIO_ReadPin(bUpperOctave_GPIO_Port, bUpperOctave_Pin)){
	osc1.frequency = 783.99;
	osc1.phaseIncrement = computePhaseIncrement(osc1.frequency, &hi2s3);
   }


  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : bLowerOctave_Pin bUpperOctave_Pin bsquare_Pin bsinus_Pin
                           btriangle_Pin bsaw_Pin */
  GPIO_InitStruct.Pin = bLowerOctave_Pin|bUpperOctave_Pin|bsquare_Pin|bsinus_Pin
                          |btriangle_Pin|bsaw_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 Audio_RST_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
