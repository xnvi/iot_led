/*
适配 Adafruit_NeoPixel
实现了部分接口，能满足基本使用
*/

#include "ws2812_driver.h"

#include <string.h>


#include "stm32g0xx_ll_gpio.h"
#include "user_hw_init.h"


// O0优化
// #define WS2812_DELAY1() \
// __NOP();__NOP();__NOP();__NOP();

// #define WS2812_DELAY2() \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP();

// #define WS2812_DELAY3() \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();


// O3优化
#define WS2812_DELAY1() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();

#define WS2812_DELAY2() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();

#define WS2812_DELAY3() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();

uint32_t ws2812_buf[WS2812_LED_NUMS] = {0};
uint8_t ws2812_brightness;
// uint16_t ws2812_numBytes = WS2812_LED_NUMS;
uint16_t ws2812_numBytes = sizeof(ws2812_buf);

void ws2812_init(void)
{
	// GPIO init
}

void ws2812_reset(void)
{
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_1);
	delay_us(10);
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
	delay_us(60);
}

void ws2812_write_byte(uint8_t dat)
{
	uint8_t i = 8;

	while (i)
	{
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_1);
		WS2812_DELAY1();
		if(!(dat & 0x80))
		{
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
		}
		WS2812_DELAY2();
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
		WS2812_DELAY3();

		dat <<= 1;
		i--;
	}
}

//数组按RGB顺序存储，输出GRB
void ws2812_write_color_rgb(uint8_t *color)
{
	__disable_irq();
	ws2812_write_byte(color[1]);
	ws2812_write_byte(color[0]);
	ws2812_write_byte(color[2]);
	__enable_irq();
}

//高字节忽略，剩下三字节按RGB顺序存储，输出GRB
void ws2812_write_color_u32(uint32_t color)
{
	__disable_irq();
	// 存储格式wrgb
	ws2812_write_byte((color >> 8) & 0xFF);
	ws2812_write_byte((color >> 16) & 0xFF);
	ws2812_write_byte(color & 0xFF);
	__enable_irq();

}

// 将缓存的颜色全部写入灯带
void ws2812_write_all(void)
{
	int i = 0;
	for (i = 0; i < WS2812_LED_NUMS; i++)
	{
		ws2812_write_color_u32(ws2812_buf[i]);
	}
}

void ws2812_clear(void)
{
	memset(ws2812_buf, 0, sizeof(uint32_t) * WS2812_LED_NUMS);
}

void ws2812_fill(uint32_t c, uint16_t first, uint16_t count)
{
	uint16_t i, end;

	if(first >= WS2812_LED_NUMS)
	{
		return; // If first LED is past end of strip, nothing to do
	}

	// Calculate the index ONE AFTER the last pixel to fill
	if(count == 0)
	{
		// Fill to end of strip
		end = WS2812_LED_NUMS;
	}
	else
	{
		// Ensure that the loop won't go past the last pixel
		end = first + count;
		if(end > WS2812_LED_NUMS) end = WS2812_LED_NUMS;
	}

	for(i = first; i < end; i++)
	{
		ws2812_set_color_u32(i, c);
	}
}

/*
//按wbgr顺序存储
//有白色的话高8位作为白色
//否则高8位忽略，剩下24位按RGB顺序存储，输出GRB
void ws2812_set_color_rgbw(uint32_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
	// ws2812_buf[n] = (w << 24) + (b << 16) + (g << 8) + r;
	ws2812_buf[n] = (b << 16) + (g << 8) + r;
}

void ws2812_set_color_uint32(uint32_t n, uint32_t color)
{
	ws2812_buf[n] = color;
}
*/


//按wrgb顺序存储
//有白色的话高8位作为白色
//否则高8位忽略，剩下24位按RGB顺序存储，输出GRB
void ws2812_set_color_rgbw(uint32_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
	// ws2812_buf[n] = (w << 24) + (r << 16) + (g << 8) + b;
	ws2812_buf[n] = (r << 16) + (g << 8) + b;
}

void ws2812_set_color_u32(uint32_t n, uint32_t color)
{
	ws2812_buf[n] = color;
}




















/* A PROGMEM (flash mem) table containing 8-bit unsigned sine wave (0-255).
   Copy & paste this snippet into a Python REPL to regenerate:
import math
for x in range(256):
    print("{:3},".format(int((math.sin(x/128.0*math.pi)+1.0)*127.5+0.5))),
    if x&15 == 15: print
*/
static const uint8_t _NeoPixelSineTable[256] =
{
	128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
	176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
	245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
	245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
	176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82,
	79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
	37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
	10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
	0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9,
	10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
	37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
	79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

/* Similar to above, but for an 8-bit gamma-correction table.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t _NeoPixelGammaTable[256] =
{
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
	3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
	7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
	13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
	20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
	30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
	58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
	76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
	97, 99, 100, 102, 103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120,
	122, 124, 125, 127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148,
	150, 152, 154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180,
	182, 184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
	218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252, 255
};



/*!
	@brief   Get a pointer directly to the NeoPixel data buffer in RAM.
			Pixel data is stored in a device-native format (a la the NEO_*
			constants) and is not translated here. Applications that access
			this buffer will need to be aware of the specific data format
			and handle colors appropriately.
	@return  Pointer to NeoPixel buffer (uint8_t* array).
	@note    This is for high-performance applications where calling
			setPixelColor() on every single pixel would be too slow (e.g.
			POV or light-painting projects). There is no bounds checking
			on the array, creating tremendous potential for mayhem if one
			writes past the ends of the buffer. Great power, great
			responsibility and all that.
*/
uint8_t *ws2812_getPixels(void)
{
	return (uint8_t *)ws2812_buf;
};






/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. The next call to show() will
           refresh the LEDs at this level.
  @param   b  Brightness setting, 0=minimum (off), 255=brightest.
  @note    This was intended for one-time use in one's setup() function,
           not as an animation effect in itself. Because of the way this
           library "pre-multiplies" LED colors in RAM, changing the
           brightness is often a "lossy" operation -- what you write to
           pixels isn't necessary the same as what you'll read back.
           Repeated brightness changes using this function exacerbate the
           problem. Smart programs therefore treat the strip as a
           write-only resource, maintaining their own state to render each
           frame of an animation, not relying on read-modify-write.
*/
void ws2812_setBrightness(uint8_t b)
{
	// Stored brightness value is different than what's passed.
	// This simplifies the actual scaling math later, allowing a fast
	// 8x8-bit multiply and taking the MSB. 'brightness' is a uint8_t,
	// adding 1 here may (intentionally) roll over...so 0 = max brightness
	// (color values are interpreted literally; no scaling), 1 = min
	// brightness (off), 255 = just below max brightness.
	uint8_t newBrightness = b + 1;
	if(newBrightness != ws2812_brightness)   // Compare against prior value
	{
		// Brightness has changed -- re-scale existing data in RAM,
		// This process is potentially "lossy," especially when increasing
		// brightness. The tight timing in the WS2811/WS2812 code means there
		// aren't enough free cycles to perform this scaling on the fly as data
		// is issued. So we make a pass through the existing color data in RAM
		// and scale it (subsequent graphics commands also work at this
		// brightness level). If there's a significant step up in brightness,
		// the limited number of steps (quantization) in the old data will be
		// quite visible in the re-scaled version. For a non-destructive
		// change, you'll need to re-render the full strip data. C'est la vie.
		uint8_t  c,
				 *ptr           = (uint8_t *)ws2812_buf,
				  oldBrightness = ws2812_brightness - 1; // De-wrap old brightness value
		uint16_t scale;
		if(oldBrightness == 0) scale = 0; // Avoid /0
		else if(b == 255) scale = 65535 / oldBrightness;
		else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
		for(uint16_t i = 0; i < ws2812_numBytes; i++)
		{
			c      = *ptr;
			*ptr++ = (c * scale) >> 8;
		}
		ws2812_brightness = newBrightness;
	}
}

uint8_t ws2812_getBrightness(void)
{
	return ws2812_brightness - 1;
}

/*!
	@brief   Return the number of pixels in an Adafruit_NeoPixel strip object.
	@return  Pixel count (0 if not set).
*/
uint16_t ws2812_numPixels(void)
{
	return WS2812_LED_NUMS;
}

uint32_t ws2812_getPixelColor(uint16_t n)
{
	// 注意统一颜色格式
	return ws2812_buf[n];
}

/*!
	@brief   An 8-bit integer sine wave function, not directly compatible
			with standard trigonometric units like radians or degrees.
	@param   x  Input angle, 0-255; 256 would loop back to zero, completing
				the circle (equivalent to 360 degrees or 2 pi radians).
				One can therefore use an unsigned 8-bit variable and simply
				add or subtract, allowing it to overflow/underflow and it
				still does the expected contiguous thing.
	@return  Sine result, 0 to 255, or -128 to +127 if type-converted to
			a signed int8_t, but you'll most likely want unsigned as this
			output is often used for pixel brightness in animation effects.
*/
uint8_t ws2812_sine8(uint8_t x)
{
	return _NeoPixelSineTable[x]; // 0-255 in, 0-255 out
}
/*!
	@brief   An 8-bit gamma-correction function for basic pixel brightness
			adjustment. Makes color transitions appear more perceptially
			correct.
	@param   x  Input brightness, 0 (minimum or off/black) to 255 (maximum).
	@return  Gamma-adjusted brightness, can then be passed to one of the
			setPixelColor() functions. This uses a fixed gamma correction
			exponent of 2.6, which seems reasonably okay for average
			NeoPixels in average tasks. If you need finer control you'll
			need to provide your own gamma-correction function instead.
*/
uint8_t ws2812_gamma8(uint8_t x)
{
	return _NeoPixelGammaTable[x]; // 0-255 in, 0-255 out
}
