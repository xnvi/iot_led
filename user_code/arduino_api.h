#ifndef __ARDUINO_API_H
#define __ARDUINO_API_H

#include <stdint.h>

#define arduino_min(a,b) ((a)<(b)?(a):(b))
#define arduino_max(a,b) ((a)>(b)?(a):(b))
#define arduino_abs(x) ((x)>0?(x):-(x))
#define arduino_constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

uint32_t arduino_millis(void);
void arduino_delay(uint32_t ms);
int32_t arduino_random(int32_t max);
int32_t arduino_random_at(int32_t min, int32_t max);

#endif
