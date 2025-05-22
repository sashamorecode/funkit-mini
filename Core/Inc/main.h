/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
enum effect{
	PASS,
	ECHO,
	DELAY,
	FILTER,
	NUM_EFFECTS,
};

typedef struct{
	enum effect currentEffect;
	int effectTracker; //used to make it take 2 encoder vals for change effect.

	float volume;

	int lastEncoderVal1;
	int encoder1Delta;
	int lastEncoderVal2;
	int encoder2Delta;
}masterState;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define SAMPLES           512
#define BUF_SAMPLES       SAMPLES * 4
#define	DELAY_BUF	      30000
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EncoderSW_Pin GPIO_PIN_0
#define EncoderSW_GPIO_Port GPIOG
#define Button1_Pin GPIO_PIN_1
#define Button1_GPIO_Port GPIOG
#define EncoderSW2_Pin GPIO_PIN_1
#define EncoderSW2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
#define STATE_UPDATE_DIV 1000
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
