/*
 * servo.h
 * Control 6 servos using TIM2 (CH1-4) and TIM3 (CH1-2)
 */
#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f1xx_hal.h"

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void servo_init(void);
void servo_set_us(uint8_t channel, uint16_t pulse_us);
void servo_set_angle(uint8_t channel, uint8_t angle);

#endif /* __SERVO_H */
