#include "servo.h"
#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

void MX_TIM2_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71; // 72MHz / (71+1) = 1MHz -> 1 tick = 1us
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 20000 - 1; // 20ms -> 50Hz
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim2);

  /* Configure PA0 PA1 PA2 PA3 as TIM2 CH1-4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500; // 1.5ms center
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3);
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4);
}

void MX_TIM3_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71; // 1 MHz
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 20000 - 1; // 20 ms
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim3);

  /* TIM3 CH1 PA6, CH2 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
}

void servo_init(void)
{
  MX_TIM2_Init();
  MX_TIM3_Init();

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

/* channel: 0..5
   mapping: 0->TIM2_CH1 (PA0)
            1->TIM2_CH2 (PA1)
            2->TIM2_CH3 (PA2)
            3->TIM2_CH4 (PA3)
            4->TIM3_CH1 (PA6)
            5->TIM3_CH2 (PA7)
*/
void servo_set_us(uint8_t channel, uint16_t pulse_us)
{
  if (pulse_us < 500) pulse_us = 500;
  if (pulse_us > 2500) pulse_us = 2500;

  switch (channel) {
    case 0: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse_us); break;
    case 1: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse_us); break;
    case 2: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse_us); break;
    case 3: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pulse_us); break;
    case 4: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_us); break;
    case 5: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse_us); break;
    default: break;
  }
}

void servo_set_angle(uint8_t channel, uint8_t angle)
{
  if (angle > 180) angle = 180;
  /* Map 0..180 -> 1000..2000 us */
  uint16_t us = 1000 + (uint32_t)angle * 1000 / 180;
  servo_set_us(channel, us);
}
