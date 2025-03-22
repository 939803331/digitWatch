#ifndef __COUNTDOWN_H
#define __COUNTDOWN_H

#include "stm32f1xx_hal.h"

typedef enum
{
    STATE_IDLE,        // 就绪状态：倒计时已设置好，等待开始倒计时
    STATE_SETTING_MIN, // 正在设置分钟（第一、二位闪烁）
    STATE_SETTING_SEC, // 正在设置秒钟（第三、四位闪烁）
    STATE_COUNTDOWN,   // 正在倒计时
    STATE_STOP,        // 暂停
    STATE_FINISHED     // 倒计时结束，闪烁显示全0
} CountdownState;

extern volatile CountdownState countdownState;

void display_countdown(uint8_t d[4], uint8_t flag);

void run_countdown();

void setting_countdown(uint16_t GPIO_Pin);

void blink_control_countdown();

void redCountdownTime();

#endif