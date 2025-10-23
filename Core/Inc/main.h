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
#include "stm32g4xx_hal.h"

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
#define USER_R_Pin GPIO_PIN_0
#define USER_R_GPIO_Port GPIOC
#define USER_G_Pin GPIO_PIN_1
#define USER_G_GPIO_Port GPIOC
#define USER_B_Pin GPIO_PIN_2
#define USER_B_GPIO_Port GPIOC
#define HSADC1_Pin GPIO_PIN_3
#define HSADC1_GPIO_Port GPIOA
#define HSEN1_Pin GPIO_PIN_4
#define HSEN1_GPIO_Port GPIOA
#define HSDIAG1_Pin GPIO_PIN_5
#define HSDIAG1_GPIO_Port GPIOA
#define HSDIAG2_Pin GPIO_PIN_6
#define HSDIAG2_GPIO_Port GPIOA
#define HSEN2_Pin GPIO_PIN_7
#define HSEN2_GPIO_Port GPIOA
#define HSADC2_Pin GPIO_PIN_4
#define HSADC2_GPIO_Port GPIOC
#define HSEN3_Pin GPIO_PIN_5
#define HSEN3_GPIO_Port GPIOC
#define HSDIAG3_Pin GPIO_PIN_0
#define HSDIAG3_GPIO_Port GPIOB
#define HSDIAG4_Pin GPIO_PIN_1
#define HSDIAG4_GPIO_Port GPIOB
#define HSEN4_Pin GPIO_PIN_2
#define HSEN4_GPIO_Port GPIOB
#define I_AUX2_Pin GPIO_PIN_11
#define I_AUX2_GPIO_Port GPIOB
#define F_ALERT2_Pin GPIO_PIN_12
#define F_ALERT2_GPIO_Port GPIOB
#define F_ALERT1_Pin GPIO_PIN_13
#define F_ALERT1_GPIO_Port GPIOB
#define I_AUX1_Pin GPIO_PIN_14
#define I_AUX1_GPIO_Port GPIOB
#define I_AUX2B15_Pin GPIO_PIN_15
#define I_AUX2B15_GPIO_Port GPIOB
#define CAN_FLT_Pin GPIO_PIN_9
#define CAN_FLT_GPIO_Port GPIOA
#define TERM_EN_Pin GPIO_PIN_10
#define TERM_EN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} HSEN_Pin_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
