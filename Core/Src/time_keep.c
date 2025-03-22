#include "time_keep.h"
#include "main.h"

uint32_t elapsedTime = 0; // elapsedTime 单位为毫秒
uint8_t isTiming = 0;     // 是否正在计时

void addElapsedTime()
{
    elapsedTime += 1;
}

void display_time_keep(uint8_t d[4], uint8_t flag)
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
        if (flag == 1)
        {
            // 显示毫秒小数点
            HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        else
        {
            // 显示时钟冒号
            HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        HAL_Delay(2);
    }
}

void run_time_keep()
{
    uint8_t d[4] = {0}; // 四个数字，分别对应数码管左到右
    // 格式标识
    uint8_t flag = 0;

    if (elapsedTime < 10UL * 60 * 1000) // 小于10分钟
    {
        // 计算：分钟为整分钟数（个位），秒为余下的秒数，毫秒这里用 100 毫秒级（即取 elapsedTime%1000/100）
        uint8_t minutes = elapsedTime / (60 * 1000);          // 0 ~ 9
        uint8_t seconds = (elapsedTime % (60 * 1000)) / 1000; // 0 ~ 59
        uint8_t ms_digit = (elapsedTime % 1000) / 100;        // 0 ~ 9（显示100ms的档位）

        d[0] = minutes;      // 第一位：分钟
        d[1] = seconds / 10; // 秒的十位
        d[2] = seconds % 10; // 秒的个位
        d[3] = ms_digit;     // 100ms档位
        flag = 1;
    }
    else if (elapsedTime < 60UL * 60 * 1000) // 10分钟到60分钟之间
    {
        // 直接以 mm:ss 格式显示
        uint8_t minutes = elapsedTime / (60 * 1000);          // 10 ~ 59
        uint8_t seconds = (elapsedTime % (60 * 1000)) / 1000; // 0 ~ 59

        d[0] = minutes / 10; // 分钟十位
        d[1] = minutes % 10; // 分钟个位
        d[2] = seconds / 10; // 秒钟十位
        d[3] = seconds % 10; // 秒钟个位
        flag = 2;
    }
    else // 60分钟及以上，显示 hh:mm（最大 99:59）
    {
        uint16_t totalMinutes = elapsedTime / (60 * 1000);
        uint8_t hours = totalMinutes / 60; // 小时数
        if (hours > 99)
            hours = 99;                      // 最大99小时
        uint8_t minutes = totalMinutes % 60; // 分钟

        d[0] = hours / 10;   // 小时十位
        d[1] = hours % 10;   // 小时个位
        d[2] = minutes / 10; // 分钟十位
        d[3] = minutes % 10; // 分钟个位
        flag = 3;
    }

    display_time_keep(d, flag);
}

void setting_time_keep(uint16_t GPIO_Pin)
{
    // 消抖：200ms内不重复响应
    if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
    {
        elapsedTime = 0; // 清零计时
        isTiming = 0;    // 暂停计时
    }
    else if (GPIO_Pin == BUTTON_INTERRUPT_2_Pin)
    {
        isTiming = !isTiming; // 开始 / 暂停 计时
    }
}