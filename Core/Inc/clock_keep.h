#ifndef __CLOCK_KEEP_H
#define __CLOCK_KEEP_H

#include "stm32f1xx_hal.h"

void display_clock(uint8_t d[4]);

void run_clock_keep();

void Blink_Control();

void setting_clock_keep(uint16_t GPIO_Pin);

void Local_SetTime();

void RTC_SetTime(uint8_t hour, uint8_t min);

#endif