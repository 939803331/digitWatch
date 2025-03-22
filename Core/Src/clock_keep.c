#include "clock_keep.h"
#include "main.h"
#include "rtc.h"

uint8_t setting_mode = 0;      // 0: 正常模式, 1: 设置小时, 2: 设置分钟
uint8_t hours = 12;            // 当前小时
uint8_t minutes = 0;           // 当前分钟
uint8_t blink_flag = 0;        // 闪烁标志

void display_clock(uint8_t d[4])
{
    // 将 d 分别显示到 4 位共阴数码管上
    for (uint8_t i = 0; i < DIGIT_COUNT; i++)
    {
        HAL_GPIO_WritePin(GPIOA, DIGIT_1_Pin | DIGIT_2_Pin | DIGIT_3_Pin | DIGIT_4_Pin, GPIO_PIN_SET);

        HAL_GPIO_WritePin(GPIOA, DIGIT_ARR[i], GPIO_PIN_RESET);

        uint8_t code = segment_code[d[i]];
        for (uint8_t j = 0; j < 7; j++)
        {
            HAL_GPIO_WritePin(GPIOA, LED_ARR[j], ((code >> j) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        // 显示时钟冒号
        HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        HAL_Delay(2);
    }
}
void run_clock_keep()
{
    uint8_t d[4] = {0};
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    if (setting_mode == 1)
    { // 小时设置中
        if (blink_flag)
        {
            d[0] = hours / 10;
            d[1] = hours % 10;
            d[2] = minutes / 10;
            d[3] = minutes % 10;
        }
        else
        {
            // 隐藏小时
            d[0] = 10;
            d[1] = 10;
            d[2] = minutes / 10;
            d[3] = minutes % 10;
        }
    }
    else if (setting_mode == 2)
    { // 分钟设置中
        if (blink_flag)
        {
            d[0] = hours / 10;
            d[1] = hours % 10;
            d[2] = minutes / 10;
            d[3] = minutes % 10;
        }
        else
        {
            d[0] = hours / 10;
            d[1] = hours % 10;
            // 隐藏分钟
            d[2] = 10;
            d[3] = 10;
        }
    }
    else
    {
        // 正常显示
        d[0] = sTime.Hours / 10;
        d[1] = sTime.Hours % 10;
        d[2] = sTime.Minutes / 10;
        d[3] = sTime.Minutes % 10;
    }
    display_clock(d);

    // 闪烁
    Blink_Control();
}

void Blink_Control()
{
    static uint32_t last_blink_time = 0;
    if (HAL_GetTick() - last_blink_time >= 500)
    {
        blink_flag = !blink_flag; // 切换闪烁状态
        last_blink_time = HAL_GetTick();
    }
}

void setting_clock_keep(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
    {
        if (setting_mode == 0)
        {
            setting_mode = 1; // 进入小时设置
            Local_SetTime();  // 同步RTC时钟
        }
        else if (setting_mode == 1)
        {
            setting_mode = 2; // 进入分钟设置
        }
        else
        {
            setting_mode = 0; // 退出设置模式，保存时间
            RTC_SetTime(hours, minutes);
        }
    }
    else if (GPIO_Pin == BUTTON_INTERRUPT_2_Pin)
    {
        if (setting_mode == 1)
        {
            hours = (hours + 1) % 24; // 小时加 1（0~23）
        }
        else if (setting_mode == 2)
        {
            minutes = (minutes + 1) % 60; // 分钟加 1（0~59）
        }
    }
}

void Local_SetTime()
{
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    hours = sTime.Hours;
    minutes = sTime.Minutes;
}
void RTC_SetTime(uint8_t hour, uint8_t min)
{
    RTC_TimeTypeDef sTime = {0};

    sTime.Hours = hour;
    sTime.Minutes = min;
    sTime.Seconds = 0;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}