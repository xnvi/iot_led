/*
适配 arduino.h 中的部分接口
*/

#include "arduino_api.h"

// #include <string.h>
#include <stdlib.h>

// #include "stm32g0xx_ll_gpio.h"
// #include "user_hw_init.h"
#include "tos_k.h"

uint32_t arduino_millis(void)
{
	return HAL_GetTick();
}

void arduino_delay(uint32_t ms)
{
	tos_sleep_ms(ms);
	// tos_task_delay(tos_millisec2tick(31));
}

int32_t arduino_random(int32_t max)
{
	srand(arduino_millis());
	return rand() % max;
}

int32_t arduino_random_at(int32_t min, int32_t max)
{
	srand(arduino_millis());
	return min + rand() % (max - min);
}
