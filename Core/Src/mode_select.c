#include "mode_select.h"
#include "main.h"

volatile uint32_t ms_lastPB0Time = 0; // PB0按钮消抖
volatile uint32_t ms_lastPA7Time = 0; // PA7按钮消抖

void display_mode_select(uint8_t d[4])
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

        HAL_Delay(2);
    }
}

void setting_mode_select(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
    {
        // 读取当前PB0电平（假设低电平代表按下，上拉方式，所以高电平代表释放）
        GPIO_PinState state = HAL_GPIO_ReadPin(GPIOB, BUTTON_INTERRUPT_1_Pin);

        if (state == GPIO_PIN_RESET) // 按钮按下（低电平）
        {
            // 进入对应模式
            switch (modeNumber)
            {
            case 1:
                modeState = MODE_STATE_TIMER;
                break;
            case 2:
                modeState = MODE_STATE_CLOCK;
                break;
            case 3:
                modeState = MODE_STATE_COUNTDOWN;
                break;
            case 4:
                modeState = MODE_STATE_DICE;
                break;
            case 5:
                modeState = MODE_STATE_STANDBY;
                isInStandby = 1;
                break;
            default:
                break;
            }
        }
    }
    else if (GPIO_Pin == BUTTON_INTERRUPT_2_Pin)
    {
        // 滚动显示下一个模式编号
        modeNumber++;
        if (modeNumber > 5)
            modeNumber = 1;
    }
}

void run_mode_select()
{
    uint8_t d[4] = {0};
    // 显示模式编号
    d[0] = modeNumber / 10;
    d[1] = modeNumber % 10;
    d[2] = 10; // 不显示
    d[3] = 10; // 不显示
    display_mode_select(d);
}
