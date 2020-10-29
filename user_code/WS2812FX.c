/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library

  NOTES
    * Uses the Adafruit NeoPixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel



  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG

  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-26   implemented segment and reverse features
  2017-11-16   changed speed calc, reduced memory footprint
  2018-02-24   added hooks for user created custom effects
*/

#include "WS2812FX.h"
#include "ws2812_driver.h"
#include "arduino_api.h"
#include <string.h>


segment WS2812FX_segments[MAX_NUM_SEGMENTS];
uint8_t WS2812FX_active_segments[MAX_NUM_ACTIVE_SEGMENTS];
segment_runtime WS2812FX_segment_runtimes[MAX_NUM_ACTIVE_SEGMENTS];


bool _running;
bool _triggered;

segment* _segments;                 // array of segments (20 bytes per element)
segment_runtime* _segment_runtimes; // array of segment runtimes (16 bytes per element)
uint8_t* _active_segments;          // array of active segments (1 bytes per element)

uint8_t _segments_len;          // size of _segments array
uint8_t _active_segments_len;   // size of _segments_runtime and _active_segments arrays
uint8_t _num_segments;          // number of configured segments in the _segments array

segment* _seg;                      // currently active segment (20 bytes)
segment_runtime* _seg_rt;           // currently active segment runtime (16 bytes)

uint16_t _seg_len;                  // num LEDs in the currently active segment
uint16_t _rand16seed;



void WS2812FX_init(void)
{
	ws2812_reset();
	
	// max_num_segments = MAX_NUM_SEGMENTS;
	// max_num_active_segments = MAX_NUM_ACTIVE_SEGMENTS;

	// brightness = DEFAULT_BRIGHTNESS + 1; // Adafruit_NeoPixel internally offsets brightness by 1
	ws2812_setBrightness(DEFAULT_BRIGHTNESS);
	_running = false;

	_segments_len = MAX_NUM_SEGMENTS;
	_active_segments_len = MAX_NUM_ACTIVE_SEGMENTS;

	// create all the segment arrays and init to zeros
	// _segments = new segment[_segments_len]();
	// _active_segments = new uint8_t[_active_segments_len]();
	// _segment_runtimes = new segment_runtime[_active_segments_len]();

	_segments = WS2812FX_segments;
	_active_segments = WS2812FX_active_segments;
	_segment_runtimes = WS2812FX_segment_runtimes;

	// init segment pointers
	_seg     = _segments;
	_seg_rt  = _segment_runtimes;

	WS2812FX_resetSegments();
	WS2812FX_setSegment_color_opt(0, 0, WS2812_LED_NUMS - 1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}



// void WS2812FX_init()
// {
// 	WS2812FX_resetSegmentRuntimes();
// 	Adafruit_NeoPixel::begin();
// }

// void WS2812FX_timer() {
//     for (int j=0; j < 1000; j++) {
//       uint16_t delay = (this->*_modes[_seg->mode])();
//     }
// }

void WS2812FX_service()
{
	if(_running || _triggered)
	{
		unsigned long now = arduino_millis(); // Be aware, millis() rolls over every 49 days
		bool doShow = false;
		for(uint8_t i = 0; i < _active_segments_len; i++)
		{
			if(_active_segments[i] != INACTIVE_SEGMENT)
			{
				_seg     = &_segments[_active_segments[i]];
				_seg_len = (uint16_t)(_seg->stop - _seg->start + 1);
				_seg_rt  = &_segment_runtimes[i];
				CLR_FRAME_CYCLE;
				if(now > _seg_rt->next_time || _triggered)
				{
					SET_FRAME;
					doShow = true;
					uint16_t delay = _modes[_seg->mode]();
					_seg_rt->next_time = now + arduino_max(delay, SPEED_MIN);
					_seg_rt->counter_mode_call++;
				}
			}
		}
		if(doShow)
		{
			arduino_delay(1); // for ESP32 (see https://forums.adafruit.com/viewtopic.php?f=47&t=117327)
			ws2812_write_all();
		}
		_triggered = false;
	}
}

// overload setPixelColor() functions so we can use gamma correction
// (see https://learn.adafruit.com/led-tricks-gamma-correction/the-issue)
void WS2812FX_setPixelColor_u32(uint16_t n, uint32_t c)
{
	// uint8_t w = (c >> 24) & 0xFF;
	// uint8_t r = (c >> 16) & 0xFF;
	// uint8_t g = (c >>  8) & 0xFF;
	// uint8_t b =  c        & 0xFF;
	// WS2812FX_setPixelColor_rgbw(n, r, g, b, w);
	ws2812_set_color_u32(n, c);
}

void WS2812FX_setPixelColor_rgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
	WS2812FX_setPixelColor_rgbw(n, r, g, b, 0);
}

void WS2812FX_setPixelColor_rgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
	// if(IS_GAMMA)
	// {
	// 	Adafruit_NeoPixel::setPixelColor(n, gamma8(r), gamma8(g), gamma8(b), gamma8(w));
	// }
	// else
	// {
	// 	Adafruit_NeoPixel::setPixelColor(n, r, g, b, w);
	// }
	ws2812_set_color_rgbw(n, r, g, b, w);
}

void WS2812FX_copyPixels(uint16_t dest, uint16_t src, uint16_t count)
{
	uint8_t *pixels = ws2812_getPixels();
	uint8_t bytesPerPixel = WS2812FX_getNumBytesPerPixel(); // 3=RGB, 4=RGBW

	memmove(pixels + (dest * bytesPerPixel), pixels + (src * bytesPerPixel), count * bytesPerPixel);
}

// change the underlying Adafruit_NeoPixel pixels pointer (use with care)
void WS2812FX_setPixels(uint16_t num_leds, uint8_t* ptr)
{
	// TODO 这个功能暂时不实现，以后再说
	// free(Adafruit_NeoPixel::pixels); // free existing data (if any)
	// Adafruit_NeoPixel::pixels = ptr;
	// Adafruit_NeoPixel::numLEDs = num_leds;
	// Adafruit_NeoPixel::numBytes = num_leds * ((wOffset == rOffset) ? 3 : 4);
}

// overload show() functions so we can use custom show()
void WS2812FX_show(void)
{
	// customShow == NULL ? Adafruit_NeoPixel::show() : customShow();

	// TODO 暂时不支持 custom 自定义效果，以后再说
	ws2812_write_all();
}

void WS2812FX_start()
{
	WS2812FX_resetSegmentRuntimes();
	_running = true;
}

void WS2812FX_stop()
{
	_running = false;
	WS2812FX_strip_off();
}

void WS2812FX_pause()
{
	_running = false;
}

void WS2812FX_resume()
{
	_running = true;
}

void WS2812FX_trigger()
{
	_triggered = true;
}

void WS2812FX_setMode_param1(uint8_t m)
{
	WS2812FX_setMode_param2(0, m);
}

void WS2812FX_setMode_param2(uint8_t seg, uint8_t m)
{
	WS2812FX_resetSegmentRuntime(seg);
	_segments[seg].mode = arduino_constrain(m, 0, MODE_COUNT - 1);
}

void WS2812FX_setOptions(uint8_t seg, uint8_t o)
{
	_segments[seg].options = o;
}

void WS2812FX_setSpeed_param1(uint16_t s)
{
	WS2812FX_setSpeed_param2(0, s);
}

void WS2812FX_setSpeed_param2(uint8_t seg, uint16_t s)
{
	_segments[seg].speed = arduino_constrain(s, SPEED_MIN, SPEED_MAX);
}

void WS2812FX_increaseSpeed(uint8_t s)
{
	uint16_t newSpeed = arduino_constrain(_seg->speed + s, SPEED_MIN, SPEED_MAX);
	WS2812FX_setSpeed_param1(newSpeed);
}

void WS2812FX_decreaseSpeed(uint8_t s)
{
	uint16_t newSpeed = arduino_constrain(_seg->speed - s, SPEED_MIN, SPEED_MAX);
	WS2812FX_setSpeed_param1(newSpeed);
}

void WS2812FX_setColor_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	WS2812FX_setColor_u32(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX_setColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
	WS2812FX_setColor_u32((((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b)));
}

void WS2812FX_setColor_u32(uint32_t c)
{
	WS2812FX_setColor_seg(0, c);
}

void WS2812FX_setColor_seg(uint8_t seg, uint32_t c)
{
	_segments[seg].colors[0] = c;
}

void WS2812FX_setColors(uint8_t seg, uint32_t* c)
{
	for(uint8_t i = 0; i < MAX_NUM_COLORS; i++)
	{
		_segments[seg].colors[i] = c[i];
	}
}

void WS2812FX_setBrightness(uint8_t b)
{
	b = arduino_constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
	// Adafruit_NeoPixel::setBrightness(b);
	// Adafruit_NeoPixel::show();
	ws2812_setBrightness(b);
	ws2812_write_all();
}

void WS2812FX_increaseBrightness(uint8_t s)
{
	s = arduino_constrain(ws2812_getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
	WS2812FX_setBrightness(s);
}

void WS2812FX_decreaseBrightness(uint8_t s)
{
	s = arduino_constrain(ws2812_getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
	WS2812FX_setBrightness(s);
}

// 不打算实现
/*
void WS2812FX_setLength(uint16_t b)
{
	WS2812FX_resetSegmentRuntimes();
	if(b < 1) b = 1;

	// Decrease numLEDs to maximum available memory
	do
	{
		Adafruit_NeoPixel::updateLength(b);
		b--;
	}
	while(!Adafruit_NeoPixel::numLEDs && b > 1);

	_segments[0].start = 0;
	_segments[0].stop = Adafruit_NeoPixel::numLEDs - 1;
}

void WS2812FX_increaseLength(uint16_t s)
{
	uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
	setLength(seglen + s);
}

void WS2812FX_decreaseLength(uint16_t s)
{
	uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
	fill(BLACK, _segments[0].start, seglen);
	show();

	if(s < seglen) setLength(seglen - s);
}
*/

bool WS2812FX_isRunning()
{
	return _running;
}

bool WS2812FX_isTriggered()
{
	return _triggered;
}

bool WS2812FX_isFrame_void()
{
	return WS2812FX_isFrame_u8(0);
}

bool WS2812FX_isFrame_u8(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr == NULL) return false; // segment not active
	return (_segment_runtimes[ptr - _active_segments].aux_param2 & FRAME);
}

bool WS2812FX_isCycle_void()
{
	return WS2812FX_isCycle_u8(0);
}

bool WS2812FX_isCycle_u8(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr == NULL) return false; // segment not active
	return (_segment_runtimes[ptr - _active_segments].aux_param2 & CYCLE);
}

void WS2812FX_setCycle()
{
	SET_CYCLE;
}

uint8_t WS2812FX_getMode_void(void)
{
	return WS2812FX_getMode_u8(0);
}

uint8_t WS2812FX_getMode_u8(uint8_t seg)
{
	return _segments[seg].mode;
}

uint16_t WS2812FX_getSpeed_void(void)
{
	return WS2812FX_getSpeed_u8(0);
}

uint16_t WS2812FX_getSpeed_u8(uint8_t seg)
{
	return _segments[seg].speed;
}

uint8_t WS2812FX_getOptions(uint8_t seg)
{
	return _segments[seg].options;
}

uint16_t WS2812FX_getLength(void)
{
	return ws2812_numPixels();
}

uint16_t WS2812FX_getNumBytes(void)
{
	// return Adafruit_NeoPixel::numBytes;
	// TODO 这个函数好像没用，以后用到再说
	return ws2812_numBytes;
}

uint8_t WS2812FX_getNumBytesPerPixel(void)
{
	// return (Adafruit_NeoPixel::wOffset == Adafruit_NeoPixel::rOffset) ? 3 : 4; // 3=RGB, 4=RGBW

	// 注意！这里写死固定4字节
	return 4;
}

uint8_t WS2812FX_getModeCount(void)
{
	return MODE_COUNT;
}

uint8_t WS2812FX_getNumSegments(void)
{
	return _num_segments;
}

void WS2812FX_setNumSegments(uint8_t n)
{
	_num_segments = n;
}

uint32_t WS2812FX_getColor_void(void)
{
	return WS2812FX_getColor_u8(0);
}

uint32_t WS2812FX_getColor_u8(uint8_t seg)
{
	return _segments[seg].colors[0];
}

uint32_t* WS2812FX_getColors(uint8_t seg)
{
	return _segments[seg].colors;
}

segment* WS2812FX_getSegment_void(void)
{
	return _seg;
}

segment* WS2812FX_getSegment_u8(uint8_t seg)
{
	return &_segments[seg];
}

segment* WS2812FX_getSegments(void)
{
	return _segments;
}

segment_runtime* WS2812FX_getSegmentRuntime_void(void)
{
	return _seg_rt;
}

segment_runtime* WS2812FX_getSegmentRuntime_u8(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr == NULL) return NULL; // segment not active
	return &_segment_runtimes[ptr - _active_segments];
}

segment_runtime* WS2812FX_getSegmentRuntimes(void)
{
	return _segment_runtimes;
}

uint8_t* WS2812FX_getActiveSegments(void)
{
	return _active_segments;
}

const char* WS2812FX_getModeName(uint8_t m)
{
	if(m < MODE_COUNT)
	{
		return _names[m];
	}
	else
	{
		// return F("");
		return NULL;
	}
}

void WS2812FX_setIdleSegment_color_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options)
{
	uint32_t colors[] = {color, 0, 0};
	WS2812FX_setIdleSegment_colors_opt(n, start, stop, mode, colors, speed, options);
}

void WS2812FX_setIdleSegment_colors_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options)
{
	WS2812FX_setSegment_colors_opt(n, start, stop, mode, colors, speed, options);
	if(n < _active_segments_len) WS2812FX_removeActiveSegment(n);;
}

void WS2812FX_setSegment_color_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse)
{
	uint32_t colors[] = {color, 0, 0};
	WS2812FX_setSegment_colors_opt(n, start, stop, mode, colors, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX_setSegment_colors_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse)
{
	WS2812FX_setSegment_colors_opt(n, start, stop, mode, colors, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX_setSegment_color_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options)
{
	uint32_t colors[] = {color, 0, 0};
	WS2812FX_setSegment_colors_opt(n, start, stop, mode, colors, speed, options);
}

void WS2812FX_setSegment_colors_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options)
{
	if(n < _segments_len)
	{
		if(n + 1 > _num_segments) _num_segments = n + 1;
		_segments[n].start = start;
		_segments[n].stop = stop;
		_segments[n].mode = mode;
		_segments[n].speed = speed;
		_segments[n].options = options;

		WS2812FX_setColors(n, (uint32_t*)colors);

		if(n < _active_segments_len) WS2812FX_addActiveSegment(n);
	}
}

void WS2812FX_addActiveSegment(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr != NULL) return; // segment already active
	for(uint8_t i = 0; i < _active_segments_len; i++)
	{
		if(_active_segments[i] == INACTIVE_SEGMENT)
		{
			_active_segments[i] = seg;
			WS2812FX_resetSegmentRuntime(seg);
			break;
		}
	}
}

void WS2812FX_removeActiveSegment(uint8_t seg)
{
	for(uint8_t i = 0; i < _active_segments_len; i++)
	{
		if(_active_segments[i] == seg)
		{
			_active_segments[i] = INACTIVE_SEGMENT;
		}
	}
}

void WS2812FX_swapActiveSegment(uint8_t oldSeg, uint8_t newSeg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, newSeg, _active_segments_len);
	if(ptr != NULL) return; // if newSeg is already active, don't swap
	for(uint8_t i = 0; i < _active_segments_len; i++)
	{
		if(_active_segments[i] == oldSeg)
		{
			_active_segments[i] = newSeg;

			// reset all runtime parameters EXCEPT next_time,
			// allowing the current animation frame to complete
			segment_runtime seg_rt = _segment_runtimes[i];
			seg_rt.counter_mode_step = 0;
			seg_rt.counter_mode_call = 0;
			seg_rt.aux_param = 0;
			seg_rt.aux_param2 = 0;
			seg_rt.aux_param3 = 0;
			break;
		}
	}
}

bool WS2812FX_isActiveSegment(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr != NULL) return true;
	return false;
}

void WS2812FX_resetSegments()
{
	WS2812FX_resetSegmentRuntimes();
	memset(_segments, 0, _segments_len * sizeof(segment));
	memset(_active_segments, INACTIVE_SEGMENT, _active_segments_len);
	_num_segments = 0;
}

void WS2812FX_resetSegmentRuntimes()
{
	memset(_segment_runtimes, 0, _active_segments_len * sizeof(segment_runtime));
}

void WS2812FX_resetSegmentRuntime(uint8_t seg)
{
	uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
	if(ptr == NULL) return; // segment not active
	memset(&_segment_runtimes[ptr - _active_segments], 0, sizeof(segment_runtime));
}

/*
 * Turns everything off. Doh.
 */
void WS2812FX_strip_off()
{
	ws2812_clear();
	ws2812_write_all();
}

/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX_color_wheel(uint8_t pos)
{
	pos = 255 - pos;
	if(pos < 85)
	{
		return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
	}
	else if(pos < 170)
	{
		pos -= 85;
		return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
	}
	else
	{
		pos -= 170;
		return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
	}
}

/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX_get_random_wheel_index(uint8_t pos)
{
	uint8_t r = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t d = 0;

	while(d < 42)
	{
		r = WS2812FX_random8_void();
		x = arduino_abs(pos - r);
		y = 255 - x;
		d = arduino_min(x, y);
	}

	return r;
}

// fast 8-bit random number generator shamelessly borrowed from FastLED
uint8_t WS2812FX_random8_void()
{
	_rand16seed = (_rand16seed * 2053) + 13849;
	return (uint8_t)((_rand16seed + (_rand16seed >> 8)) & 0xFF);
}

// note random8(lim) generates numbers in the range 0 to (lim -1)
uint8_t WS2812FX_random8_u8(uint8_t lim)
{
	uint8_t r = WS2812FX_random8_void();
	r = ((uint16_t)r * lim) >> 8;
	return r;
}

uint16_t WS2812FX_random16_void()
{
	return (uint16_t)WS2812FX_random8_void() * 256 + WS2812FX_random8_void();
}

// note random16(lim) generates numbers in the range 0 to (lim - 1)
uint16_t WS2812FX_random16_u16(uint16_t lim)
{
	uint16_t r = WS2812FX_random16_void();
	r = ((uint32_t)r * lim) >> 16;
	return r;
}

// Return the sum of all LED intensities (can be used for
// rudimentary power calculations)
uint32_t WS2812FX_intensitySum()
{
	uint8_t *pixels = ws2812_getPixels();
	uint32_t sum = 0;
	for(uint16_t i = 0; i < ws2812_numBytes; i++)
	{
		sum += pixels[i];
	}
	return sum;
}

// Return the sum of each color's intensity. Note, the order of
// intensities in the returned array depends on the type of WS2812
// LEDs you have. NEO_GRB LEDs will return an array with entries
// in a different order then NEO_RGB LEDs.
uint32_t* WS2812FX_intensitySums()
{
	static uint32_t intensities[] = { 0, 0, 0, 0 };
	memset(intensities, 0, sizeof(intensities));

	uint8_t *pixels = ws2812_getPixels();
	uint8_t bytesPerPixel = WS2812FX_getNumBytesPerPixel(); // 3=RGB, 4=RGBW
	for(uint16_t i = 0; i < ws2812_numBytes; i += bytesPerPixel)
	{
		intensities[0] += pixels[i];
		intensities[1] += pixels[i + 1];
		intensities[2] += pixels[i + 2];
		if(bytesPerPixel == 4) intensities[3] += pixels[i + 3]; // for RGBW LEDs
	}
	return intensities;
}


/* #####################################################
#
#  Mode Functions
#
##################################################### */

/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX_mode_static(void)
{
	ws2812_fill(_seg->colors[0], _seg->start, _seg_len);
	SET_CYCLE;
	return _seg->speed;
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX_blink(uint32_t color1, uint32_t color2, bool strobe)
{
	if(_seg_rt->counter_mode_call & 1)
	{
		uint32_t color = (IS_REVERSE) ? color1 : color2; // off
		ws2812_fill(color, _seg->start, _seg_len);
		SET_CYCLE;
		return strobe ? _seg->speed - 20 : (_seg->speed / 2);
	}
	else
	{
		uint32_t color = (IS_REVERSE) ? color2 : color1; // on
		ws2812_fill(color, _seg->start, _seg_len);
		return strobe ? 20 : (_seg->speed / 2);
	}
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX_mode_blink(void)
{
	return WS2812FX_blink(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX_mode_blink_rainbow(void)
{
	return WS2812FX_blink(WS2812FX_color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX_mode_strobe(void)
{
	return WS2812FX_blink(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX_mode_strobe_rainbow(void)
{
	return WS2812FX_blink(WS2812FX_color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], true);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX_color_wipe(uint32_t color1, uint32_t color2, bool rev)
{
	if(_seg_rt->counter_mode_step < _seg_len)
	{
		uint32_t led_offset = _seg_rt->counter_mode_step;
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - led_offset, color1);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + led_offset, color1);
		}
	}
	else
	{
		uint32_t led_offset = _seg_rt->counter_mode_step - _seg_len;
		if((IS_REVERSE && !rev) || (!IS_REVERSE && rev))
		{
			WS2812FX_setPixelColor_u32(_seg->stop - led_offset, color2);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + led_offset, color2);
		}
	}

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len * 2);

	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

	return (_seg->speed / (_seg_len * 2));
}

/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX_mode_color_wipe(void)
{
	return WS2812FX_color_wipe(_seg->colors[0], _seg->colors[1], false);
}

uint16_t WS2812FX_mode_color_wipe_inv(void)
{
	return WS2812FX_color_wipe(_seg->colors[1], _seg->colors[0], false);
}

uint16_t WS2812FX_mode_color_wipe_rev(void)
{
	return WS2812FX_color_wipe(_seg->colors[0], _seg->colors[1], true);
}

uint16_t WS2812FX_mode_color_wipe_rev_inv(void)
{
	return WS2812FX_color_wipe(_seg->colors[1], _seg->colors[0], true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX_mode_color_wipe_random(void)
{
	if(_seg_rt->counter_mode_step % _seg_len == 0)   // aux_param will store our random color wheel index
	{
		_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
	}
	uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
	return WS2812FX_color_wipe(color, color, false) * 2;
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX_mode_color_sweep_random(void)
{
	if(_seg_rt->counter_mode_step % _seg_len == 0)   // aux_param will store our random color wheel index
	{
		_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
	}
	uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
	return WS2812FX_color_wipe(color, color, true) * 2;
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX_mode_random_color(void)
{
	_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param); // aux_param will store our random color wheel index
	uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
	ws2812_fill(color, _seg->start, _seg_len);
	SET_CYCLE;
	return _seg->speed;
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 */
uint16_t WS2812FX_mode_single_dynamic(void)
{
	if(_seg_rt->counter_mode_call == 0)
	{
		for(uint16_t i = _seg->start; i <= _seg->stop; i++)
		{
			WS2812FX_setPixelColor_u32(i, WS2812FX_color_wheel(WS2812FX_random8_void()));
		}
	}

	WS2812FX_setPixelColor_u32(_seg->start + WS2812FX_random16_u16(_seg_len), WS2812FX_color_wheel(WS2812FX_random8_void()));
	SET_CYCLE;
	return _seg->speed;
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX_mode_multi_dynamic(void)
{
	for(uint16_t i = _seg->start; i <= _seg->stop; i++)
	{
		WS2812FX_setPixelColor_u32(i, WS2812FX_color_wheel(WS2812FX_random8_void()));
	}
	SET_CYCLE;
	return _seg->speed;
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX_mode_breath(void)
{
	int lum = _seg_rt->counter_mode_step;
	if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

	uint16_t delay;
	if(lum == 15) delay = 970; // 970 pause before each breath
	else if(lum <=  25) delay = 38; // 19
	else if(lum <=  50) delay = 36; // 18
	else if(lum <=  75) delay = 28; // 14
	else if(lum <= 100) delay = 20; // 10
	else if(lum <= 125) delay = 14; // 7
	else if(lum <= 150) delay = 11; // 5
	else delay = 10; // 4

	uint32_t color =  WS2812FX_color_blend(_seg->colors[1], _seg->colors[0], lum);
	ws2812_fill(color, _seg->start, _seg_len);

	_seg_rt->counter_mode_step += 2;
	if(_seg_rt->counter_mode_step > (512 - 15))
	{
		_seg_rt->counter_mode_step = 15;
		SET_CYCLE;
	}
	return delay;
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX_mode_fade(void)
{
	int lum = _seg_rt->counter_mode_step;
	if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0

	uint32_t color = WS2812FX_color_blend(_seg->colors[1], _seg->colors[0], lum);
	ws2812_fill(color, _seg->start, _seg_len);

	_seg_rt->counter_mode_step += 4;
	if(_seg_rt->counter_mode_step > 511)
	{
		_seg_rt->counter_mode_step = 0;
		SET_CYCLE;
	}
	return (_seg->speed / 128);
}


/*
 * scan function - runs a block of pixels back and forth.
 */
uint16_t WS2812FX_scan(uint32_t color1, uint32_t color2, bool dual)
{
	int8_t dir = _seg_rt->aux_param ? -1 : 1;
	uint8_t size = 1 << SIZE_OPTION;

	ws2812_fill(color2, _seg->start, _seg_len);

	for(uint8_t i = 0; i < size; i++)
	{
		if(IS_REVERSE || dual)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - _seg_rt->counter_mode_step - i, color1);
		}
		if(!IS_REVERSE || dual)
		{
			WS2812FX_setPixelColor_u32(_seg->start + _seg_rt->counter_mode_step + i, color1);
		}
	}

	_seg_rt->counter_mode_step += dir;
	if(_seg_rt->counter_mode_step == 0)
	{
		_seg_rt->aux_param = 0;
		SET_CYCLE;
	}
	if(_seg_rt->counter_mode_step >= (uint16_t)(_seg_len - size)) _seg_rt->aux_param = 1;

	return (_seg->speed / (_seg_len * 2));
}


/*
 * Runs a block of pixels back and forth.
 */
uint16_t WS2812FX_mode_scan(void)
{
	return WS2812FX_scan(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Runs two blocks of pixels back and forth in opposite directions.
 */
uint16_t WS2812FX_mode_dual_scan(void)
{
	return WS2812FX_scan(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX_mode_rainbow(void)
{
	uint32_t color = WS2812FX_color_wheel(_seg_rt->counter_mode_step);
	ws2812_fill(color, _seg->start, _seg_len);

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

	if(_seg_rt->counter_mode_step == 0)  SET_CYCLE;

	return (_seg->speed / 256);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX_mode_rainbow_cycle(void)
{
	for(uint16_t i = 0; i < _seg_len; i++)
	{
		uint32_t color = WS2812FX_color_wheel(((i * 256 / _seg_len) + _seg_rt->counter_mode_step) & 0xFF);
		WS2812FX_setPixelColor_u32(_seg->start + i, color);
	}

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

	return (_seg->speed / 256);
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX_tricolor_chase(uint32_t color1, uint32_t color2, uint32_t color3)
{
	uint8_t sizeCnt = 1 << SIZE_OPTION;
	uint8_t sizeCnt2 = sizeCnt + sizeCnt;
	uint8_t sizeCnt3 = sizeCnt2 + sizeCnt;
	uint16_t index = _seg_rt->counter_mode_step % sizeCnt3;
	for(uint16_t i = 0; i < _seg_len; i++, index++)
	{
		index = index % sizeCnt3;

		uint32_t color = color3;
		if(index < sizeCnt) color = color1;
		else if(index < sizeCnt2) color = color2;

		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->start + i, color);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->stop - i, color);
		}
	}

	_seg_rt->counter_mode_step++;
	if(_seg_rt->counter_mode_step % _seg_len == 0) SET_CYCLE;

	return (_seg->speed / _seg_len);
}


/*
 * Tricolor chase mode
 */
uint16_t WS2812FX_mode_tricolor_chase(void)
{
	return WS2812FX_tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX_mode_circus_combustus(void)
{
	return WS2812FX_tricolor_chase(RED, WHITE, BLACK);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX_mode_theater_chase(void)
{
	return WS2812FX_tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[1]);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX_mode_theater_chase_rainbow(void)
{
	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;
	uint32_t color = WS2812FX_color_wheel(_seg_rt->counter_mode_step);
	return WS2812FX_tricolor_chase(color, _seg->colors[1], _seg->colors[1]);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX_mode_running_lights(void)
{
	uint8_t size = 1 << SIZE_OPTION;
	uint8_t sineIncr = arduino_max(1, (256 / _seg_len) * size);
	for(uint16_t i = 0; i < _seg_len; i++)
	{
		int lum = (int)ws2812_sine8(((i + _seg_rt->counter_mode_step) * sineIncr));
		uint32_t color = WS2812FX_color_blend(_seg->colors[0], _seg->colors[1], lum);
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->start + i, color);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->stop - i,  color);
		}
	}
	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % 256;
	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
	return (_seg->speed / _seg_len);
}


/*
 * twinkle function
 */
uint16_t WS2812FX_twinkle(uint32_t color1, uint32_t color2)
{
	if(_seg_rt->counter_mode_step == 0)
	{
		ws2812_fill(color2, _seg->start, _seg_len);
		uint16_t min_leds = (_seg_len / 4) + 1; // make sure, at least one LED is on
		_seg_rt->counter_mode_step = arduino_random_at(min_leds, min_leds * 2);
		SET_CYCLE;
	}

	WS2812FX_setPixelColor_u32(_seg->start + WS2812FX_random16_u16(_seg_len), color1);

	_seg_rt->counter_mode_step--;
	return (_seg->speed / _seg_len);
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_twinkle(void)
{
	return WS2812FX_twinkle(_seg->colors[0], _seg->colors[1]);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_twinkle_random(void)
{
	return WS2812FX_twinkle(WS2812FX_color_wheel(WS2812FX_random8_void()), _seg->colors[1]);
}


/*
 * fade out functions
 */
void WS2812FX_fade_out_void()
{
	// return WS2812FX_fade_out_u32(_seg->colors[1]);
	WS2812FX_fade_out_u32(_seg->colors[1]);
}

void WS2812FX_fade_out_u32(uint32_t targetColor)
{
	static const uint8_t rateMapH[] = {0, 1, 1, 1, 2, 3, 4, 6};
	static const uint8_t rateMapL[] = {0, 2, 3, 8, 8, 8, 8, 8};

	uint8_t rate  = FADE_RATE;
	uint8_t rateH = rateMapH[rate];
	uint8_t rateL = rateMapL[rate];

	uint32_t color = targetColor;
	int w2 = (color >> 24) & 0xff;
	int r2 = (color >> 16) & 0xff;
	int g2 = (color >>  8) & 0xff;
	int b2 =  color        & 0xff;

	for(uint16_t i = _seg->start; i <= _seg->stop; i++)
	{
		// color = Adafruit_NeoPixel::getPixelColor(i); // current color
		color = ws2812_getPixelColor(i); // current color
		if(rate == 0)   // old fade-to-black algorithm
		{
			WS2812FX_setPixelColor_u32(i, (color >> 1) & 0x7F7F7F7F);
		}
		else     // new fade-to-color algorithm
		{
			int w1 = (color >> 24) & 0xff;
			int r1 = (color >> 16) & 0xff;
			int g1 = (color >>  8) & 0xff;
			int b1 =  color        & 0xff;

			// calculate the color differences between the current and target colors
			int wdelta = w2 - w1;
			int rdelta = r2 - r1;
			int gdelta = g2 - g1;
			int bdelta = b2 - b1;

			// if the current and target colors are almost the same, jump right to the target
			// color, otherwise calculate an intermediate color. (fixes rounding issues)
			wdelta = arduino_abs(wdelta) < 3 ? wdelta : (wdelta >> rateH) + (wdelta >> rateL);
			rdelta = arduino_abs(rdelta) < 3 ? rdelta : (rdelta >> rateH) + (rdelta >> rateL);
			gdelta = arduino_abs(gdelta) < 3 ? gdelta : (gdelta >> rateH) + (gdelta >> rateL);
			bdelta = arduino_abs(bdelta) < 3 ? bdelta : (bdelta >> rateH) + (bdelta >> rateL);

			WS2812FX_setPixelColor_rgbw(i, r1 + rdelta, g1 + gdelta, b1 + bdelta, w1 + wdelta);
		}
	}
}


/*
 * color blend function
 */
uint32_t WS2812FX_color_blend(uint32_t color1, uint32_t color2, uint8_t blend)
{
	if(blend == 0)   return color1;
	if(blend == 255) return color2;

	uint8_t w1 = (color1 >> 24) & 0xff;
	uint8_t r1 = (color1 >> 16) & 0xff;
	uint8_t g1 = (color1 >>  8) & 0xff;
	uint8_t b1 =  color1        & 0xff;

	uint8_t w2 = (color2 >> 24) & 0xff;
	uint8_t r2 = (color2 >> 16) & 0xff;
	uint8_t g2 = (color2 >>  8) & 0xff;
	uint8_t b2 =  color2        & 0xff;

	uint32_t w3 = ((w2 * blend) + (w1 * (255U - blend))) / 256U;
	uint32_t r3 = ((r2 * blend) + (r1 * (255U - blend))) / 256U;
	uint32_t g3 = ((g2 * blend) + (g1 * (255U - blend))) / 256U;
	uint32_t b3 = ((b2 * blend) + (b1 * (255U - blend))) / 256U;

	return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}


/*
 * twinkle_fade function
 */
uint16_t WS2812FX_twinkle_fade(uint32_t color)
{
	WS2812FX_fade_out_void();

	if(WS2812FX_random8_u8(3) == 0)
	{
		uint8_t size = 1 << SIZE_OPTION;
		uint16_t index = _seg->start + WS2812FX_random16_u16(_seg_len - size);
		ws2812_fill(color, index, size);
		SET_CYCLE;
	}
	return (_seg->speed / 8);
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX_mode_twinkle_fade(void)
{
	return WS2812FX_twinkle_fade(_seg->colors[0]);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX_mode_twinkle_fade_random(void)
{
	return WS2812FX_twinkle_fade(WS2812FX_color_wheel(WS2812FX_random8_void()));
}

/*
 * Sparkle function
 * color1 = background color
 * color2 = sparkle color
 */
uint16_t WS2812FX_sparkle(uint32_t color1, uint32_t color2)
{
	if(_seg_rt->counter_mode_step == 0)
	{
		ws2812_fill(color1, _seg->start, _seg_len);
	}

	uint8_t size = 1 << SIZE_OPTION;
	ws2812_fill(color1, _seg->start + _seg_rt->aux_param3, size);

	_seg_rt->aux_param3 = WS2812FX_random16_u16(_seg_len - size); // aux_param3 stores the random led index
	ws2812_fill(color2, _seg->start + _seg_rt->aux_param3, size);

	SET_CYCLE;
	return (_seg->speed / 32);
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_sparkle(void)
{
	return WS2812FX_sparkle(_seg->colors[1], _seg->colors[0]);
}


/*
 * Lights all LEDs in the color. Flashes white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_flash_sparkle(void)
{
	return WS2812FX_sparkle(_seg->colors[0], WHITE);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_hyper_sparkle(void)
{
	ws2812_fill(_seg->colors[0], _seg->start, _seg_len);

	uint8_t size = 1 << SIZE_OPTION;
	for(uint8_t i = 0; i < 8; i++)
	{
		ws2812_fill(WHITE, _seg->start + WS2812FX_random16_u16(_seg_len - size), size);
	}

	SET_CYCLE;
	return (_seg->speed / 32);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX_mode_multi_strobe(void)
{
	ws2812_fill(_seg->colors[1], _seg->start, _seg_len);

	uint16_t delay = 200 + ((9 - (_seg->speed % 10)) * 100);
	uint16_t count = 2 * ((_seg->speed / 100) + 1);
	if(_seg_rt->counter_mode_step < count)
	{
		if((_seg_rt->counter_mode_step & 1) == 0)
		{
			ws2812_fill(_seg->colors[0], _seg->start, _seg_len);
			delay = 20;
		}
		else
		{
			delay = 50;
		}
	}

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (count + 1);
	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
	return delay;
}


/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX_chase(uint32_t color1, uint32_t color2, uint32_t color3)
{
	uint8_t size = 1 << SIZE_OPTION;
	for(uint8_t i = 0; i < size; i++)
	{
		uint16_t a = (_seg_rt->counter_mode_step + i) % _seg_len;
		uint16_t b = (a + size) % _seg_len;
		uint16_t c = (b + size) % _seg_len;
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - a, color1);
			WS2812FX_setPixelColor_u32(_seg->stop - b, color2);
			WS2812FX_setPixelColor_u32(_seg->stop - c, color3);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + a, color1);
			WS2812FX_setPixelColor_u32(_seg->start + b, color2);
			WS2812FX_setPixelColor_u32(_seg->start + c, color3);
		}
	}

	if(_seg_rt->counter_mode_step + (size * 3) == _seg_len) SET_CYCLE;

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
	return (_seg->speed / _seg_len);
}


/*
 * Bicolor chase mode
 */
uint16_t WS2812FX_mode_bicolor_chase(void)
{
	return WS2812FX_chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * White running on _color.
 */
uint16_t WS2812FX_mode_chase_color(void)
{
	return WS2812FX_chase(_seg->colors[0], WHITE, WHITE);
}


/*
 * Black running on _color.
 */
uint16_t WS2812FX_mode_chase_blackout(void)
{
	return WS2812FX_chase(_seg->colors[0], BLACK, BLACK);
}


/*
 * _color running on white.
 */
uint16_t WS2812FX_mode_chase_white(void)
{
	return WS2812FX_chase(WHITE, _seg->colors[0], _seg->colors[0]);
}


/*
 * White running followed by random color.
 */
uint16_t WS2812FX_mode_chase_random(void)
{
	if(_seg_rt->counter_mode_step == 0)
	{
		_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
	}
	return WS2812FX_chase(WS2812FX_color_wheel(_seg_rt->aux_param), WHITE, WHITE);
}


/*
 * Rainbow running on white.
 */
uint16_t WS2812FX_mode_chase_rainbow_white(void)
{
	uint16_t n = _seg_rt->counter_mode_step;
	uint16_t m = (_seg_rt->counter_mode_step + 1) % _seg_len;
	uint32_t color2 = WS2812FX_color_wheel(((n * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);
	uint32_t color3 = WS2812FX_color_wheel(((m * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);

	return WS2812FX_chase(WHITE, color2, color3);
}


/*
 * White running on rainbow.
 */
uint16_t WS2812FX_mode_chase_rainbow(void)
{
	uint8_t color_sep = 256 / _seg_len;
	uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
	uint32_t color = WS2812FX_color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

	return WS2812FX_chase(color, WHITE, WHITE);
}


/*
 * Black running on rainbow.
 */
uint16_t WS2812FX_mode_chase_blackout_rainbow(void)
{
	uint8_t color_sep = 256 / _seg_len;
	uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
	uint32_t color = WS2812FX_color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

	return WS2812FX_chase(color, BLACK, BLACK);
}

/*
 * running white flashes function.
 * color1 = background color
 * color2 = flash color
 */
uint16_t WS2812FX_chase_flash(uint32_t color1, uint32_t color2)
{
	const static uint8_t flash_count = 4;
	uint8_t flash_step = _seg_rt->counter_mode_call % ((flash_count * 2) + 1);

	if(flash_step < (flash_count * 2))
	{
		uint32_t color = (flash_step % 2 == 0) ? color2 : color1;
		uint16_t n = _seg_rt->counter_mode_step;
		uint16_t m = (_seg_rt->counter_mode_step + 1) % _seg_len;
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - n, color);
			WS2812FX_setPixelColor_u32(_seg->stop - m, color);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + n, color);
			WS2812FX_setPixelColor_u32(_seg->start + m, color);
		}
		return 30;
	}
	else
	{
		_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
		if(_seg_rt->counter_mode_step == 0)
		{
			// update aux_param so mode_chase_flash_random() will select the next color
			_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
			SET_CYCLE;
		}
	}
	return (_seg->speed / _seg_len);
}

/*
 * White flashes running on _color.
 */
uint16_t WS2812FX_mode_chase_flash(void)
{
	return WS2812FX_chase_flash(_seg->colors[0], WHITE);
}


/*
 * White flashes running, followed by random color.
 */
uint16_t WS2812FX_mode_chase_flash_random(void)
{
	return WS2812FX_chase_flash(WS2812FX_color_wheel(_seg_rt->aux_param), WHITE);
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX_running(uint32_t color1, uint32_t color2)
{
	uint8_t size = 2 << SIZE_OPTION;
	uint32_t color = (_seg_rt->counter_mode_step & size) ? color1 : color2;

	if(IS_REVERSE)
	{
		WS2812FX_copyPixels(_seg->start, _seg->start + 1, _seg_len - 1);
		WS2812FX_setPixelColor_u32(_seg->stop, color);
	}
	else
	{
		WS2812FX_copyPixels(_seg->start + 1, _seg->start, _seg_len - 1);
		WS2812FX_setPixelColor_u32(_seg->start, color);
	}

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
	return (_seg->speed / _seg_len);
}


/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX_mode_running_color(void)
{
	return WS2812FX_running(_seg->colors[0], _seg->colors[1]);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX_mode_running_red_blue(void)
{
	return WS2812FX_running(RED, BLUE);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX_mode_merry_christmas(void)
{
	return WS2812FX_running(RED, GREEN);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX_mode_halloween(void)
{
	return WS2812FX_running(PURPLE, ORANGE);
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX_mode_running_random(void)
{
	uint8_t size = 2 << SIZE_OPTION;
	if((_seg_rt->counter_mode_step) % size == 0)
	{
		_seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
	}

	uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);

	return WS2812FX_running(color, color);
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX_mode_larson_scanner(void)
{
	WS2812FX_fade_out_void();

	if(_seg_rt->counter_mode_step < _seg_len)
	{
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
		}
	}
	else
	{
		uint16_t index = (_seg_len * 2) - _seg_rt->counter_mode_step - 2;
		if(IS_REVERSE)
		{
			WS2812FX_setPixelColor_u32(_seg->stop - index, _seg->colors[0]);
		}
		else
		{
			WS2812FX_setPixelColor_u32(_seg->start + index, _seg->colors[0]);
		}
	}

	_seg_rt->counter_mode_step++;
	if(_seg_rt->counter_mode_step >= (uint16_t)((_seg_len * 2) - 2))
	{
		_seg_rt->counter_mode_step = 0;
		SET_CYCLE;
	}

	return (_seg->speed / (_seg_len * 2));
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX_mode_comet(void)
{
	WS2812FX_fade_out_void();

	if(IS_REVERSE)
	{
		WS2812FX_setPixelColor_u32(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
	}
	else
	{
		WS2812FX_setPixelColor_u32(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
	}

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
	if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

	return (_seg->speed / _seg_len);
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX_fireworks(uint32_t color)
{
	WS2812FX_fade_out_void();

// for better performance, manipulate the Adafruit_NeoPixels pixels[] array directly
	uint8_t *pixels = ws2812_getPixels();
	uint8_t bytesPerPixel = WS2812FX_getNumBytesPerPixel(); // 3=RGB, 4=RGBW
	uint16_t startPixel = _seg->start * bytesPerPixel + bytesPerPixel;
	uint16_t stopPixel = _seg->stop * bytesPerPixel ;
	for(uint16_t i = startPixel; i < stopPixel; i++)
	{
		uint16_t tmpPixel = (pixels[i - bytesPerPixel] >> 2) +
							pixels[i] +
							(pixels[i + bytesPerPixel] >> 2);
		pixels[i] =  tmpPixel > 255 ? 255 : tmpPixel;
	}

	uint8_t size = 2 << SIZE_OPTION;
	if(!_triggered)
	{
		for(uint16_t i = 0; i < arduino_max(1, _seg_len / 20); i++)
		{
			if(WS2812FX_random8_u8(10) == 0)
			{
				uint16_t index = _seg->start + WS2812FX_random16_u16(_seg_len - size);
				ws2812_fill(color, index, size);
				SET_CYCLE;
			}
		}
	}
	else
	{
		for(uint16_t i = 0; i < arduino_max(1, _seg_len / 10); i++)
		{
			uint16_t index = _seg->start + WS2812FX_random16_u16(_seg_len - size);
			ws2812_fill(color, index, size);
			SET_CYCLE;
		}
	}

	return (_seg->speed / _seg_len);
}

/*
 * Firework sparks.
 */
uint16_t WS2812FX_mode_fireworks(void)
{
	uint32_t color = BLACK;
	do   // randomly choose a non-BLACK color from the colors array
	{
		color = _seg->colors[WS2812FX_random8_u8(MAX_NUM_COLORS)];
	}
	while(color == BLACK);
	return WS2812FX_fireworks(color);
}

/*
 * Random colored firework sparks.
 */
uint16_t WS2812FX_mode_fireworks_random(void)
{
	return WS2812FX_fireworks(WS2812FX_color_wheel(WS2812FX_random8_void()));
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX_fire_flicker(int rev_intensity)
{
	uint8_t w = (_seg->colors[0] >> 24) & 0xFF;
	uint8_t r = (_seg->colors[0] >> 16) & 0xFF;
	uint8_t g = (_seg->colors[0] >>  8) & 0xFF;
	uint8_t b = (_seg->colors[0]        & 0xFF);
	uint8_t lum = arduino_max(w, arduino_max(r, arduino_max(g, b))) / rev_intensity;
	for(uint16_t i = _seg->start; i <= _seg->stop; i++)
	{
		int flicker = WS2812FX_random8_u8(lum);
		WS2812FX_setPixelColor_rgbw(i, arduino_max(r - flicker, 0), arduino_max(g - flicker, 0), arduino_max(b - flicker, 0), arduino_max(w - flicker, 0));
	}

	SET_CYCLE;
	return (_seg->speed / _seg_len);
}

/*
 * Random flickering.
 */
uint16_t WS2812FX_mode_fire_flicker(void)
{
	return WS2812FX_fire_flicker(3);
}

/*
* Random flickering, less intensity.
*/
uint16_t WS2812FX_mode_fire_flicker_soft(void)
{
	return WS2812FX_fire_flicker(6);
}

/*
* Random flickering, more intensity.
*/
uint16_t WS2812FX_mode_fire_flicker_intense(void)
{
	return WS2812FX_fire_flicker(1);
}


/*
 * ICU mode
 */
// uint16_t WS2812FX::mode_icu(void) {
//   uint16_t dest = _seg_rt->counter_mode_step & 0xFFFF;

//   setPixelColor(_seg->start + dest, _seg->colors[0]);
//   setPixelColor(_seg->start + dest + _seg_len/2, _seg->colors[0]);

//   if(_seg_rt->aux_param3 == dest) { // pause between eye movements
//     if(random8(6) == 0) { // blink once in a while
//       setPixelColor(_seg->start + dest, BLACK);
//       setPixelColor(_seg->start + dest + _seg_len/2, BLACK);
//       return 200;
//     }
//     _seg_rt->aux_param3 = random16(_seg_len/2);
//     SET_CYCLE;
//     return 1000 + random16(2000);
//   }

//   setPixelColor(_seg->start + dest, BLACK);
//   setPixelColor(_seg->start + dest + _seg_len/2, BLACK);

//   if(_seg_rt->aux_param3 > _seg_rt->counter_mode_step) {
//     _seg_rt->counter_mode_step++;
//     dest++;
//   } else if (_seg_rt->aux_param3 < _seg_rt->counter_mode_step) {
//     _seg_rt->counter_mode_step--;
//     dest--;
//   }

//   setPixelColor(_seg->start + dest, _seg->colors[0]);
//   setPixelColor(_seg->start + dest + _seg_len/2, _seg->colors[0]);

//   return (_seg->speed / _seg_len);
// }

// 自定义模式暂时砍掉，没精力弄，以后再说（没有以后）
#if 0
/*
 * Custom modes
 */
uint16_t WS2812FX::mode_custom_0()
{
	return customModes[0]();
}
uint16_t WS2812FX::mode_custom_1()
{
	return customModes[1]();
}
uint16_t WS2812FX::mode_custom_2()
{
	return customModes[2]();
}
uint16_t WS2812FX::mode_custom_3()
{
	return customModes[3]();
}
uint16_t WS2812FX::mode_custom_4()
{
	return customModes[4]();
}
uint16_t WS2812FX::mode_custom_5()
{
	return customModes[5]();
}
uint16_t WS2812FX::mode_custom_6()
{
	return customModes[6]();
}
uint16_t WS2812FX::mode_custom_7()
{
	return customModes[7]();
}

/*
 * Custom mode helpers
 */
void WS2812FX::setCustomMode(uint16_t (*p)())
{
	customModes[0] = p;
}

uint8_t WS2812FX::setCustomMode(const __FlashStringHelper* name, uint16_t (*p)())
{
	static uint8_t custom_mode_index = 0;
	return setCustomMode(custom_mode_index++, name, p);
}

uint8_t WS2812FX::setCustomMode(uint8_t index, const __FlashStringHelper* name, uint16_t (*p)())
{
	if((uint8_t)(FX_MODE_CUSTOM_0 + index) < MODE_COUNT)
	{
		_names[FX_MODE_CUSTOM_0 + index] = name; // store the custom mode name
		customModes[index] = p; // store the custom mode

		return (FX_MODE_CUSTOM_0 + index);
	}
	return 0;
}

/*
 * Custom show helper
 */
void WS2812FX::setCustomShow(void (*p)())
{
	customShow = p;
}
#endif
