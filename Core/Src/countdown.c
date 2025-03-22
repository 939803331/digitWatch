#include "countdown.h"
#include "main.h"

volatile CountdownState countdownState = STATE_IDLE;

volatile uint8_t setMinutes = 2;     // 默认5分钟（0~60循环）
volatile uint8_t setSeconds = 0;    // 默认0秒（0~60循环）
volatile uint32_t countdownTime = 0; // 毫秒计时（倒计时剩余时间）

volatile uint8_t cd_blink_flag = 0; // 闪烁标志
uint32_t lastBlinkTime = 0;         // 用于闪烁定时（例如500ms）

void display_countdown(uint8_t d[4], uint8_t flag)
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
            // 显示时钟冒号
            HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        else if (flag == 3)
        {
            // 显示时钟冒号
            HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 1 && cd_blink_flag) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        else
        {
            // 显示毫秒小数点
            HAL_GPIO_WritePin(GPIOA, LED_DP_Pin, (i == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        HAL_Delay(2);
    }
}

void run_countdown()
{
    uint8_t d[4] = {0};
    // 格式标识
    uint8_t flag = 1;

    if (countdownState == STATE_SETTING_MIN)
    {
        // 设置分钟时，闪烁第一、二位，其他正常显示当前设置的秒数
        if (cd_blink_flag)
        {
            // 隐藏分钟
            d[0] = 10;
            d[1] = 10;
        }
        else
        {
            d[0] = setMinutes / 10;
            d[1] = setMinutes % 10;
        }
        d[2] = setSeconds / 10;
        d[3] = setSeconds % 10;
    }
    else if (countdownState == STATE_SETTING_SEC)
    {
        // 设置秒钟时，闪烁第三、四位，分钟正常显示
        d[0] = setMinutes / 10;
        d[1] = setMinutes % 10;
        if (cd_blink_flag)
        {
            // 隐藏秒钟
            d[2] = 10;
            d[3] = 10;
        }
        else
        {
            d[2] = setSeconds / 10;
            d[3] = setSeconds % 10;
        }
    }
    else if (countdownState == STATE_COUNTDOWN || countdownState == STATE_STOP)
    {
        // 倒计时运行时，根据剩余时间显示不同格式
        if (countdownTime >= 10 * 60 * 1000UL)
        { // >= 10分钟
            uint32_t totalSec = countdownTime / 1000;
            uint8_t minutes = totalSec / 60;
            uint8_t seconds = totalSec % 60;
            d[0] = minutes / 10;
            d[1] = minutes % 10;
            d[2] = seconds / 10;
            d[3] = seconds % 10;
            flag = 1;
        }
        else
        { // 10分钟以内：第一位分钟，第二、三位秒，第四位为毫秒的十分位
            uint32_t totalSec = countdownTime / 1000;
            uint8_t minute = totalSec / 60;
            uint8_t second = totalSec % 60;
            // 毫秒部分（取十分位）
            uint8_t ms = (countdownTime % 1000) / 100;
            d[0] = minute; // 注意：此处只显示个位分钟（若分钟超过9可能需调整）
            d[1] = second / 10;
            d[2] = second % 10;
            d[3] = ms;
            flag = 2;
        }
    }
    else if (countdownState == STATE_FINISHED)
    {
        // 倒计时结束，闪烁全0
        if (cd_blink_flag)
        {
            d[0] = d[1] = d[2] = d[3] = 0;
        }
        else
        {
            d[0] = d[1] = d[2] = d[3] = 10; // 不显示（闪烁）
        }
        flag = 3;
    }
    else
    { // STATE_IDLE 正常显示倒计时时间（未启动倒计时）
        d[0] = setMinutes / 10;
        d[1] = setMinutes % 10;
        d[2] = setSeconds / 10;
        d[3] = setSeconds % 10;
    }

    display_countdown(d, flag);

    // 闪烁
    blink_control_countdown();
}

void blink_control_countdown()
{
    static uint32_t last_blink_time = 0;
    if (HAL_GetTick() - last_blink_time >= 500)
    {
        cd_blink_flag = !cd_blink_flag; // 切换闪烁状态
        last_blink_time = HAL_GetTick();
    }
}

void redCountdownTime()
{
    countdownTime--;
    if (countdownTime == 0)
    {
        countdownState = STATE_FINISHED;

        // 倒计时结束，不能进入待机模式
        if (modeState == MODE_STATE_STANDBY)
        {
            modeState = MODE_STATE_COUNTDOWN;
            isInStandby = 0;
        }
    }
}

void setting_countdown(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
    {
        if (countdownState == STATE_IDLE)
        {
            // 进入设置模式：先设置分钟
            countdownState = STATE_SETTING_MIN;
        }
        else if (countdownState == STATE_SETTING_MIN)
        {
            // 保存分钟设置，进入设置秒钟
            countdownState = STATE_SETTING_SEC;
        }
        else if (countdownState == STATE_SETTING_SEC || countdownState == STATE_COUNTDOWN)
        {
            // 保存秒钟设置，退出设置，准备倒计时
            countdownState = STATE_IDLE;
        }
    }
    else if (GPIO_Pin == BUTTON_INTERRUPT_2_Pin)
    {
        if (countdownState == STATE_SETTING_MIN)
        {
            // 分钟减1，范围0～60（从0减则回到60）
            if (setMinutes == 0)
                setMinutes = 60;
            else
                setMinutes--;
        }
        else if (countdownState == STATE_SETTING_SEC)
        {
            // 秒钟减1，范围0～60（从0减则回到60）
            if (setSeconds == 0)
                setSeconds = 60;
            else
                setSeconds--;
        }
        else if (countdownState == STATE_IDLE)
        {
            // 将倒计时初始化（转换为毫秒）
            countdownTime = ((uint32_t)setMinutes * 60 + setSeconds) * 1000;
            // 在就绪状态下按PA7开始倒计时
            if (countdownTime > 0)
            {
                countdownState = STATE_COUNTDOWN;
            }
        }
        else if (countdownState == STATE_COUNTDOWN)
        {
            countdownState = STATE_STOP;
        }
        else if (countdownState == STATE_STOP)
        {
            countdownState = STATE_COUNTDOWN;
        }
        else if (countdownState == STATE_FINISHED)
        {
            // 倒计时结束时，按PA7停止闪烁并回到就绪状态
            countdownState = STATE_IDLE;
            countdownTime = ((uint32_t)setMinutes * 60 + setSeconds) * 1000;
        }
    }
}
