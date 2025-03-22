#include "dice.h"
#include "main.h"
#include <stdlib.h>

typedef enum
{
    STATE_SETTING, // 正在设置每一位数码管的数字（默认初值为6）
    STATE_READY,   // 设置完毕，等待开始摇骰子
    STATE_ROLLING, // 摇骰子中，随机生成数字（显示过程从左至右，每位固定前其它位不断循环）
    STATE_FINISHED // 摇骰子结束，所有位数字固定显示
} SystemState;

volatile SystemState systemState = STATE_READY;

// 用于设置的4位初始数字，默认全为6
volatile uint8_t setDigits[4] = {6, 6, 6, 6};
volatile uint8_t currentSettingIndex = 0; // 当前正在设置哪一位（0～3）

// 摇骰子过程中用到的变量
uint8_t finalDigits[4] = {0};    // 摇骰子最终确定的数字（随机范围 0 ~ setDigits[i]）
uint8_t currentDisplay[4] = {0}; // 当前各位显示的数字（实时更新）
uint32_t rollStartTime = 0;      // 摇骰子开始的时间
uint8_t fixedCount = 0;          // 已固定的位数（从左往右依次固定）

// 用于闪烁设置数字时的定时变量（例如 500ms 闪烁一次）
volatile uint8_t blinkFlag = 0;

void display_dice(uint8_t d[4])
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

void run_dice()
{
    // 根据状态处理摇骰子流程
    if (systemState == STATE_ROLLING)
    {
        process_rolling();
    }

    uint8_t d[4] = {0};

    if (systemState == STATE_SETTING)
    {
        // 设置状态下，当前设置位闪烁
        for (uint8_t i = 0; i < 4; i++)
        {
            if (i == currentSettingIndex && blinkFlag)
            {
                // 正在设置的位不显示（闪烁）
                currentDisplay[i] = 10;
            }
            else
            {
                currentDisplay[i] = setDigits[i];
            }
        }
        d[0] = currentDisplay[0];
        d[1] = currentDisplay[1];
        d[2] = currentDisplay[2];
        d[3] = currentDisplay[3];
    }
    else if (systemState == STATE_ROLLING)
    {
        // 摇骰子状态下：左边已固定的位显示最终数字，
        // 其余位持续更新随机数字（更新速率可调）
        for (uint8_t i = 0; i < fixedCount; i++)
        {
            currentDisplay[i] = finalDigits[i]; // 已固定
        }
        // 对未固定位，快速循环显示0~9（你可以改为生成随机数）
        for (uint8_t i = fixedCount; i < 4; i++)
        {
            currentDisplay[i] = rand() % 10;
        }
        d[0] = currentDisplay[0];
        d[1] = currentDisplay[1];
        d[2] = currentDisplay[2];
        d[3] = currentDisplay[3];
    }
    else if (systemState == STATE_FINISHED)
    {
        // 摇骰子结束后，全部显示固定数字
        d[0] = finalDigits[0];
        d[1] = finalDigits[1];
        d[2] = finalDigits[2];
        d[3] = finalDigits[3];
    }
    else
    { // STATE_READY，显示设置好的数字
        d[0] = setDigits[0];
        d[1] = setDigits[1];
        d[2] = setDigits[2];
        d[3] = setDigits[3];
    }

    display_dice(d);

    // 闪烁
    blink_control_dice();
}

void blink_control_dice()
{
    static uint32_t diceLastBlinkTime = 0;
    if (HAL_GetTick() - diceLastBlinkTime >= 500)
    {
        blinkFlag = !blinkFlag; // 切换闪烁状态
        diceLastBlinkTime = HAL_GetTick();
    }
}

void setting_dice(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_INTERRUPT_1_Pin)
    {
        if (systemState == STATE_SETTING)
        {
            // 保存当前位的设置，并切换至下一位
            if (currentSettingIndex < 3)
            {
                currentSettingIndex++;
            }
            else
            {
                // 所有位设置完毕，进入 READY 状态
                systemState = STATE_READY;
                currentSettingIndex = 0;
            }
        }
        else if (systemState == STATE_READY)
        {
            // 进入设置状态
            systemState = STATE_SETTING;
        }
    }
    else if (GPIO_Pin == BUTTON_INTERRUPT_2_Pin)
    {
        if (systemState == STATE_SETTING)
        {
            // 当前位数值加1，范围 0~9
            setDigits[currentSettingIndex] = (setDigits[currentSettingIndex] + 1) % 10;
        }
        else if (systemState == STATE_READY)
        {
            // 启动摇骰子流程
            systemState = STATE_ROLLING;
            rollStartTime = HAL_GetTick();
            fixedCount = 0;
            // 根据每个位的设置值生成随机上限：
            for (uint8_t i = 0; i < 4; i++)
            {
                // 每位最终数字在 0 ~ setDigits[i] 之间随机
                finalDigits[i] = rand() % (setDigits[i] + 1);
            }
        }
        else if (systemState == STATE_FINISHED)
        {
            // 可在最终显示状态下按 PA7 重置或者重新启动摇骰子（根据需求）
            systemState = STATE_READY; // 返回准备状态
        }
    }
}

void process_rolling(void)
{
    uint32_t now = HAL_GetTick();
    // 以1秒为间隔依次固定各位
    if (systemState == STATE_ROLLING)
    {
        if (fixedCount == 0 && (now - rollStartTime >= 2000))
        {
            // 固定第一位（缓慢降速可以在这期间更新显示频率，本例直接固定）
            fixedCount = 1;
        }
        if (fixedCount == 1 && (now - rollStartTime >= 3000))
        {
            fixedCount = 2;
        }
        if (fixedCount == 2 && (now - rollStartTime >= 4000))
        {
            fixedCount = 3;
        }
        if (fixedCount == 3 && (now - rollStartTime >= 5000))
        {
            fixedCount = 4;
            systemState = STATE_FINISHED; // 全部固定后进入结束状态
        }
    }
}
