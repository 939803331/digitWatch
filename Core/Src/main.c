/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "revolve.h"
#include "time_keep.h"
#include "clock_keep.h"
#include "countdown.h"
#include "dice.h"
#include "mode_select.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const uint8_t segment_code[11] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00000000  // 10
};

// 数码管段码定义（共阴极）
const uint8_t charMap[26] = {
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, // A-J
    0x75, 0x38, 0x37, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 0x78, // K-T
    0x3E, 0x1C, 0x2A, 0x49, 0x66, 0x5B                          // U-Z
};

const uint8_t DIGIT_COUNT = 4; // 4 位数码管

char displayBuffer[30] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ  ";
uint8_t currentIndex = 0;

uint16_t LED_ARR[7] = {LED_A_Pin, LED_B_Pin, LED_C_Pin, LED_D_Pin, LED_E_Pin, LED_F_Pin, LED_G_Pin};
uint16_t DIGIT_ARR[4] = {DIGIT_1_Pin, DIGIT_2_Pin, DIGIT_3_Pin, DIGIT_4_Pin};

// 默认-模式选择模式
volatile ModeState modeState = MODE_STATE_MODE_SELECT;

// 在进入待机前，可以保存上次的模式
volatile ModeState lastModeState = MODE_STATE_MODE_SELECT;

// 当前选择的模式编 (1~5)
volatile uint8_t modeNumber = 1;

// 记录是否处于黑屏待机状态
volatile uint8_t isInStandby = 0;

// 用于记录无操作时间（单位：秒
volatile uint16_t inactivityCounter = 0;

// 按下始时
volatile uint32_t button_press_start_time = 0;
// 按钮当前是否处于按下状态
volatile uint8_t button_pressed = 0;
// 是否已经触发了长按操
volatile uint8_t long_press_triggered = 0;

// 长按阈�（单位：ms），例如 2000ms
#define LONG_PRESS_THRESHOLD 1500

// 进入待机的无操作时间阈值（2分钟=120s）
#define INACTIVITY_THRESHOLD 30

#define DEBOUNCE_THRESHOLD_PRESS 50 // 按下防抖 50ms
#define DOUBLE_CLICK_THRESHOLD 400  // 双击间隔阈值 400ms

// 全局变量，用于双击判断
volatile uint8_t clickCount = 0;      // 记录点击次数
volatile uint32_t firstClickTime = 0; // 第一次点击的时间
volatile uint32_t buttonLastTime = 0; // 防抖参考时间

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

uint8_t getSegmentCode(char c)
{
  if (c >= 'A' && c <= 'Z')
  {
    return charMap[c - 'A'];
  }
  return 0x00;
}

// 刷新 4 位数码管
void updateDisplay()
{
  for (uint8_t i = 0; i < DIGIT_COUNT; i++)
  {
    uint8_t charCode = getSegmentCode(displayBuffer[currentIndex + i]);

    HAL_GPIO_WritePin(GPIOA, DIGIT_1_Pin | DIGIT_2_Pin | DIGIT_3_Pin | DIGIT_4_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOA, DIGIT_ARR[i], GPIO_PIN_RESET);

    for (uint8_t j = 0; j < 7; j++)
    {
      HAL_GPIO_WritePin(GPIOA, LED_ARR[j], ((charCode >> j) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    HAL_Delay(2);
  }
}

// 滚动文本
void scrollText()
{
  currentIndex++;
  if (currentIndex > 26)
  {
    currentIndex = 0;
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_RTC_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // 根据 currentMode 做不同的处理
    switch (modeState)
    {
    case MODE_STATE_MODE_SELECT:
      run_mode_select();
      break;
    case MODE_STATE_TIMER:
      run_time_keep(); // 计时模式逻辑
      break;
    case MODE_STATE_CLOCK:
      run_clock_keep(); // 时钟模式逻辑
      break;
    case MODE_STATE_COUNTDOWN:
      run_countdown(); // 倒计时模
      break;
    case MODE_STATE_DICE:
      run_dice(); // 骰子模式逻辑
      break;
    case MODE_STATE_STANDBY:
      // 待机模式下黑屏，不显示任何数
      run_revolve();
      break;
    default:
      break;
    }

    // run_time_keep();

    // run_clock_keep();

    // run_countdown();

    // run_dice();

    // revolve();
  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void process_short_press(uint16_t GPIO_Pin)
{
  // 骰子模式
  // setting_dice(GPIO_Pin);
  // 倒计时模
  // setting_countdown(GPIO_Pin);
  // 时钟模式
  // setting_clock_keep(GPIO_Pin);
  //   // 计时模式
  //   // begin_end_time_keep();

  switch (modeState)
  {
  case MODE_STATE_TIMER:
    setting_time_keep(GPIO_Pin); // 计时模式逻辑
    break;
  case MODE_STATE_CLOCK:
    setting_clock_keep(GPIO_Pin); // 时钟模式逻辑
    break;
  case MODE_STATE_COUNTDOWN:
    setting_countdown(GPIO_Pin); // 倒计时
    break;
  case MODE_STATE_DICE:
    setting_dice(GPIO_Pin); // 骰子模式逻辑
    break;
  case MODE_STATE_STANDBY:
    if (isInStandby)
    {
      // 恢复到之前的模式
      modeState = lastModeState;
      isInStandby = 0;
    }
    break;
  default:
    break;
  }
}
void setting_others(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
  {

    uint32_t currentTime = HAL_GetTick();

    if (clickCount == 0)
    {
      // 第一次点击：记录时间并计数
      clickCount = 1;
      firstClickTime = currentTime;
    }
    else if (clickCount == 1)
    {
      // 如果第二次点击在双击阈值内，则直接判定为双击
      if ((currentTime - firstClickTime) <= DOUBLE_CLICK_THRESHOLD)
      {
        clickCount = 0; // 重置点击计数
        firstClickTime = 0;
        modeState = MODE_STATE_MODE_SELECT; // 处理双击操作（例如退出当前模式返回模式选择页面）
        return;
      }
      else
      {
        clickCount = 1;
        firstClickTime = currentTime;
      }
    }
  }
  // 处理短按操作
  process_short_press(GPIO_Pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // 防抖
  uint32_t currentTime = HAL_GetTick();
  if (currentTime - buttonLastTime < 200)
    return;
  buttonLastTime = currentTime;

  // 任何action 重置无操作计数器
  inactivityCounter = 0;

  // 根据 currentMode 做不同的处理
  if (modeState == MODE_STATE_MODE_SELECT)
  {
    setting_mode_select(GPIO_Pin);
  }
  else
  {
    setting_others(GPIO_Pin);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim != &htim4)
    return;
  // 1) 无操作计数
  inactivityCounter = isInStandby ? 0 : (inactivityCounter + 1);
  if (!isInStandby && inactivityCounter >= INACTIVITY_THRESHOLD)
  {
    // 如果当前模式为倒计时，并且 倒计时已经结束，则不进入待机模式
    if (modeState == MODE_STATE_COUNTDOWN && countdownState == STATE_FINISHED)
    {
      return;
    }
    // 超过2分钟无操作，进入待机模式
    lastModeState = modeState;
    modeState = MODE_STATE_STANDBY;
    isInStandby = 1;
  }
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

#ifdef USE_FULL_ASSERT
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
