#ifndef __MODE_SELECT_H
#define __MODE_SELECT_H

#include "stm32f1xx_hal.h"

void run_mode_select();

void display_mode_select(uint8_t d[4]);

void setting_mode_select(uint16_t GPIO_Pin);

#endif