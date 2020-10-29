#ifndef __USER_HW_INIT_H
#define __USER_HW_INIT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim17;

void MX_TIM3_Init(void);
void MX_TIM17_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);



void delay_us(uint32_t n);

void User_Init(void);


void HSL2RGB(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b);

void ws2812_reset(void);
void ws2812_write_byte(uint8_t dat);
void ws2812_write_color(uint8_t *color);
void ws2812_write_color_uint(uint32_t dat);

void User_GPIO_Init(void);

#ifdef __cplusplus
}
#endif
#endif
