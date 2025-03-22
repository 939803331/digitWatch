#ifndef __TIME_KEEP_H
#define __TIME_KEEP_H

#include "stm32f1xx_hal.h"

extern uint32_t elapsedTime;   // elapsedTime 单位为毫秒
extern uint8_t isTiming;       // 计时状态（0=暂停，1=运行）

void addElapsedTime();

void run_time_keep();

void setting_time_keep(uint16_t GPIO_Pin);

void display_time_keep(uint8_t d[4], uint8_t flag);



#endif