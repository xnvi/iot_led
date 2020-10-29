#ifndef __WS2812_DRIVER_H
#define __WS2812_DRIVER_H

#include <stdint.h>

#define WS2812_LED_NUMS 60

extern uint32_t ws2812_buf[];
extern uint16_t ws2812_numBytes;

void ws2812_init(void);
void ws2812_reset(void);

void ws2812_write_byte(uint8_t dat);
void ws2812_write_color_rgb(uint8_t *color);
void ws2812_write_color_u32(uint32_t color);
void ws2812_write_all(void);

void ws2812_clear(void);
void ws2812_fill(uint32_t c, uint16_t first, uint16_t count);
void ws2812_set_color_rgbw(uint32_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void ws2812_set_color_u32(uint32_t n, uint32_t color);


uint8_t *ws2812_getPixels(void);
void ws2812_setBrightness(uint8_t b);
uint8_t ws2812_getBrightness(void);
uint16_t ws2812_numPixels(void);
uint32_t ws2812_getPixelColor(uint16_t n);
uint8_t ws2812_sine8(uint8_t x);
uint8_t ws2812_gamma8(uint8_t x);


#endif
