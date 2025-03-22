#include "revolve.h"
#include "main.h"
#include <math.h>

#define DIGIT_LEN 12
// 旋转灯位
const uint16_t DIGIT[12] = {
    DIGIT_1_Pin, DIGIT_2_Pin, DIGIT_3_Pin, DIGIT_4_Pin, DIGIT_4_Pin, DIGIT_4_Pin, DIGIT_4_Pin,
    DIGIT_3_Pin, DIGIT_2_Pin, DIGIT_1_Pin, DIGIT_1_Pin, DIGIT_1_Pin};

// 旋转灯段
const uint8_t CODE[12] = {
    0x01, 0x01, 0x01, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08, 0x08, 0x10, 0x20};

void display_revolve(uint32_t Delay)
{
    for (uint8_t i = 0; i < DIGIT_LEN; i++)
    {

        HAL_GPIO_WritePin(GPIOA, DIGIT_1_Pin | DIGIT_2_Pin | DIGIT_3_Pin | DIGIT_4_Pin, GPIO_PIN_SET);

        HAL_GPIO_WritePin(GPIOA, DIGIT[i], GPIO_PIN_RESET);

        for (uint8_t j = 0; j < 7; j++)
        {
            HAL_GPIO_WritePin(GPIOA, LED_ARR[j], ((CODE[i] >> j) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        if (isInStandby) {
            HAL_Delay(Delay);
        }
    }
}

// 模式2
void run_revolve()
{
    float k = 0.3;  // 控制加速快慢的参数（越大加速越快）
    float step = 0; // 变化步长
    uint32_t Delay;

    // 加速阶段：Delay 由 100 迅速降到 10
    for (step = 0; step < 10; step++)
    {
        Delay = 100 * exp(-k * step); // 指数衰减
        if (Delay < 10)
            Delay = 10; // 最小延迟限制
        display_revolve(Delay);
    }

    // 减速阶段：Delay 由 10 逐渐回升到 100
    for (step = 10; step > 0; step--)
    {
        Delay = 100 * exp(-k * step);
        if (Delay < 10)
            Delay = 10;
        display_revolve(Delay);
    }
}

// 模式2
// void run_revolve()
// {
//     uint32_t Delay;
//     uint32_t step;

//     // 加速阶段：Delay 由 100 迅速降到 10
//     for (step = 0; step < 10; step++)
//     {
//         Delay = 100 - (step * step);  // 二次曲线加速
//         if (Delay < 10) Delay = 10;   // 限制最小延迟
//         run(Delay);
//     }

//     // 减速阶段：Delay 由 10 逐渐回升到 100
//     for (step = 10; step > 0; step--)
//     {
//         Delay = 100 - (step * step);
//         if (Delay < 10) Delay = 10;
//         run(Delay);
//     }
// }
