#include "bluetooth.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include "servo.h"

UART_HandleTypeDef huart1;
static uint8_t rx_byte;
static char line_buf[96];
static uint8_t line_idx = 0;

void MX_USART1_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PA9 TX AF_PP, PA10 RX Input */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  if (huart->Instance == USART1) {
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}

void bluetooth_init(void)
{
  MX_USART1_UART_Init();
  line_idx = 0;
  memset(line_buf,0,sizeof(line_buf));
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    char c = (char)rx_byte;
    if (c == '\r') {
      /* ignore */
    } else if (c == '\n') {
      line_buf[line_idx] = '\0';
      char *p = line_buf;
      char *tok = strtok(p, ", \t");
      int legacy_ch = 0;
      while (tok != NULL) {
        while (*tok == ' ') tok++;
        if (*tok == 'P' || *tok == 'p') {
          char *sep = strchr(tok, ':');
          if (sep) {
            *sep = '\0';
            int ch = atoi(tok + 1);
            int adc = atoi(sep + 1);
            if (ch >= 1 && ch <= 6) {
              if (adc < 0) adc = 0;
              if (adc > 4095) adc = 4095;
              uint8_t angle = (uint8_t)((adc * 180) / 4095);
              servo_set_angle((uint8_t)(ch - 1), angle);
            }
          }
        } else {
          int val = atoi(tok);
          if (val >= 0 && val <= 4095) {
            uint8_t angle = (uint8_t)((val * 180) / 4095);
            servo_set_angle((uint8_t)legacy_ch, angle);
            legacy_ch++;
          }
        }
        tok = strtok(NULL, ", \t");
      }
      line_idx = 0;
      memset(line_buf,0,sizeof(line_buf));
    } else {
      if (line_idx < (sizeof(line_buf)-1)) {
        line_buf[line_idx++] = c;
      } else {
        line_idx = 0;
      }
    }

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}
