#ifndef __DICE_H
#define __DICE_H

#include "stm32f1xx_hal.h"

void display_dice(uint8_t d[4]);

void run_dice();

void setting_dice(uint16_t GPIO_Pin);

void blink_control_dice();

void process_rolling();

#endif