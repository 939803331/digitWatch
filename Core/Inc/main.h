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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef enum {
  MODE_STATE_MODE_SELECT,  // 模式选择页面
  MODE_STATE_TIMER,        // 计时
  MODE_STATE_CLOCK,        // 时钟
  MODE_STATE_COUNTDOWN,    // 倒计时
  MODE_STATE_DICE,         // 骰子
  MODE_STATE_STANDBY      // 手动进入待机 or 自动进入待机
} ModeState;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

extern const uint8_t segment_code[11];

extern uint16_t LED_ARR[7];

extern uint16_t DIGIT_ARR[4];

extern const uint8_t DIGIT_COUNT;

extern volatile ModeState modeState;

extern volatile ModeState lastModeState;

extern volatile uint8_t modeNumber;

extern volatile uint8_t isInStandby;

extern volatile uint16_t inactivityCounter;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_E_Pin GPIO_PIN_1
#define LED_E_GPIO_Port GPIOA
#define LED_D_Pin GPIO_PIN_2
#define LED_D_GPIO_Port GPIOA
#define LED_DP_Pin GPIO_PIN_3
#define LED_DP_GPIO_Port GPIOA
#define LED_C_Pin GPIO_PIN_4
#define LED_C_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_5
#define LED_G_GPIO_Port GPIOA
#define DIGIT_4_Pin GPIO_PIN_6
#define DIGIT_4_GPIO_Port GPIOA
#define BUTTON_INTERRUPT_2_Pin GPIO_PIN_7
#define BUTTON_INTERRUPT_2_GPIO_Port GPIOA
#define BUTTON_INTERRUPT_2_EXTI_IRQn EXTI9_5_IRQn
#define BUTTON_INTERRUPT_1_Pin GPIO_PIN_0
#define BUTTON_INTERRUPT_1_GPIO_Port GPIOB
#define BUTTON_INTERRUPT_1_EXTI_IRQn EXTI0_IRQn
#define LED_B_Pin GPIO_PIN_8
#define LED_B_GPIO_Port GPIOA
#define DIGIT_3_Pin GPIO_PIN_9
#define DIGIT_3_GPIO_Port GPIOA
#define DIGIT_2_Pin GPIO_PIN_10
#define DIGIT_2_GPIO_Port GPIOA
#define LED_F_Pin GPIO_PIN_11
#define LED_F_GPIO_Port GPIOA
#define LED_A_Pin GPIO_PIN_12
#define LED_A_GPIO_Port GPIOA
#define DIGIT_1_Pin GPIO_PIN_15
#define DIGIT_1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
