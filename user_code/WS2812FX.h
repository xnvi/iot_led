/*
  WS2812FX.h - Library for WS2812 LED effects.

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
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#include "ws2812_driver.h"
#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_BRIGHTNESS (uint8_t)50
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint16_t)1000
#define DEFAULT_COLOR      (uint32_t)0xFF0000
#define DEFAULT_COLORS     { RED, GREEN, BLUE }
#define COLORS(...)        (const uint32_t[]){__VA_ARGS__}

#if defined(ESP8266) || defined(ESP32)
//#pragma message("Compiling for ESP")
#define SPEED_MIN (uint16_t)2
#else
//#pragma message("Compiling for Arduino")
#define SPEED_MIN (uint16_t)10
#endif
#define SPEED_MAX (uint16_t)65535

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255

/* each segment uses 36 bytes of SRAM memory, so if you're compile fails
  because of insufficient flash memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS         10
#define MAX_NUM_ACTIVE_SEGMENTS  10
#define INACTIVE_SEGMENT        255 /* max uint_8 */
#define MAX_NUM_COLORS            3 /* number of colors per segment */
#define MAX_CUSTOM_MODES          8

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define GRAY       (uint32_t)0x101010
#define ULTRAWHITE (uint32_t)0xFFFFFFFF
#define DIM(c)     (uint32_t)((c >> 2) & 0x3f3f3f3f) // color at 25% intensity
#define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f) // color at  6% intensity


// segment options
// bit    7: reverse animation
// bits 4-6: fade rate (0-7)
// bit    3: gamma correction
// bits 1-2: size
// bits   0: TBD
/*
#define NO_OPTIONS   (uint8_t)B00000000
#define REVERSE      (uint8_t)B10000000
#define IS_REVERSE   ((_seg->options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)B00010000
#define FADE_FAST    (uint8_t)B00100000
#define FADE_MEDIUM  (uint8_t)B00110000
#define FADE_SLOW    (uint8_t)B01000000
#define FADE_XSLOW   (uint8_t)B01010000
#define FADE_XXSLOW  (uint8_t)B01100000
#define FADE_GLACIAL (uint8_t)B01110000
#define FADE_RATE    ((_seg->options >> 4) & 7)
#define GAMMA        (uint8_t)B00001000
#define IS_GAMMA     ((_seg->options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)B00000000
#define SIZE_MEDIUM  (uint8_t)B00000010
#define SIZE_LARGE   (uint8_t)B00000100
#define SIZE_XLARGE  (uint8_t)B00000110
#define SIZE_OPTION  ((_seg->options >> 1) & 3)
*/

#define NO_OPTIONS   (uint8_t)0x00
#define REVERSE      (uint8_t)0x80
#define IS_REVERSE   ((_seg->options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)0x10
#define FADE_FAST    (uint8_t)0x20
#define FADE_MEDIUM  (uint8_t)0x30
#define FADE_SLOW    (uint8_t)0x40
#define FADE_XSLOW   (uint8_t)0x50
#define FADE_XXSLOW  (uint8_t)0x60
#define FADE_GLACIAL (uint8_t)0x70
#define FADE_RATE    ((_seg->options >> 4) & 7)
#define GAMMA        (uint8_t)0x08
#define IS_GAMMA     ((_seg->options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)0x00
#define SIZE_MEDIUM  (uint8_t)0x02
#define SIZE_LARGE   (uint8_t)0x04
#define SIZE_XLARGE  (uint8_t)0x06
#define SIZE_OPTION  ((_seg->options >> 1) & 3)

// segment runtime options (aux_param2)
// #define FRAME           (uint8_t)B10000000
#define FRAME           (uint8_t)0x80
#define SET_FRAME       (_seg_rt->aux_param2 |=  FRAME)
#define CLR_FRAME       (_seg_rt->aux_param2 &= ~FRAME)
// #define CYCLE           (uint8_t)B01000000
#define CYCLE           (uint8_t)0x40
#define SET_CYCLE       (_seg_rt->aux_param2 |=  CYCLE)
#define CLR_CYCLE       (_seg_rt->aux_param2 &= ~CYCLE)
#define CLR_FRAME_CYCLE (_seg_rt->aux_param2 &= ~(FRAME | CYCLE))

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_INV           4
#define FX_MODE_COLOR_WIPE_REV           5
#define FX_MODE_COLOR_WIPE_REV_INV       6
#define FX_MODE_COLOR_WIPE_RANDOM        7
#define FX_MODE_RANDOM_COLOR             8
#define FX_MODE_SINGLE_DYNAMIC           9
#define FX_MODE_MULTI_DYNAMIC           10
#define FX_MODE_RAINBOW                 11
#define FX_MODE_RAINBOW_CYCLE           12
#define FX_MODE_SCAN                    13
#define FX_MODE_DUAL_SCAN               14
#define FX_MODE_FADE                    15
#define FX_MODE_THEATER_CHASE           16
#define FX_MODE_THEATER_CHASE_RAINBOW   17
#define FX_MODE_RUNNING_LIGHTS          18
#define FX_MODE_TWINKLE                 19
#define FX_MODE_TWINKLE_RANDOM          20
#define FX_MODE_TWINKLE_FADE            21
#define FX_MODE_TWINKLE_FADE_RANDOM     22
#define FX_MODE_SPARKLE                 23
#define FX_MODE_FLASH_SPARKLE           24
#define FX_MODE_HYPER_SPARKLE           25
#define FX_MODE_STROBE                  26
#define FX_MODE_STROBE_RAINBOW          27
#define FX_MODE_MULTI_STROBE            28
#define FX_MODE_BLINK_RAINBOW           29
#define FX_MODE_CHASE_WHITE             30
#define FX_MODE_CHASE_COLOR             31
#define FX_MODE_CHASE_RANDOM            32
#define FX_MODE_CHASE_RAINBOW           33
#define FX_MODE_CHASE_FLASH             34
#define FX_MODE_CHASE_FLASH_RANDOM      35
#define FX_MODE_CHASE_RAINBOW_WHITE     36
#define FX_MODE_CHASE_BLACKOUT          37
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  38
#define FX_MODE_COLOR_SWEEP_RANDOM      39
#define FX_MODE_RUNNING_COLOR           40
#define FX_MODE_RUNNING_RED_BLUE        41
#define FX_MODE_RUNNING_RANDOM          42
#define FX_MODE_LARSON_SCANNER          43
#define FX_MODE_COMET                   44
#define FX_MODE_FIREWORKS               45
#define FX_MODE_FIREWORKS_RANDOM        46
#define FX_MODE_MERRY_CHRISTMAS         47
#define FX_MODE_FIRE_FLICKER            48
#define FX_MODE_FIRE_FLICKER_SOFT       49
#define FX_MODE_FIRE_FLICKER_INTENSE    50
#define FX_MODE_CIRCUS_COMBUSTUS        51
#define FX_MODE_HALLOWEEN               52
#define FX_MODE_BICOLOR_CHASE           53
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_CUSTOM                  55  // keep this for backward compatiblity
#define FX_MODE_CUSTOM_0                55  // custom modes need to go at the end
#define FX_MODE_CUSTOM_1                56
#define FX_MODE_CUSTOM_2                57
#define FX_MODE_CUSTOM_3                58
#define FX_MODE_CUSTOM_4                59
#define FX_MODE_CUSTOM_5                60
#define FX_MODE_CUSTOM_6                61
#define FX_MODE_CUSTOM_7                62

/*
// create GLOBAL names to allow WS2812FX to compile with sketches and other libs
// that store strings in PROGMEM (get rid of the "section type conflict with __c"
// errors once and for all. Amen.)
const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Breath";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Inverse";
const char name_5[] PROGMEM = "Color Wipe Reverse";
const char name_6[] PROGMEM = "Color Wipe Reverse Inverse";
const char name_7[] PROGMEM = "Color Wipe Random";
const char name_8[] PROGMEM = "Random Color";
const char name_9[] PROGMEM = "Single Dynamic";
const char name_10[] PROGMEM = "Multi Dynamic";
const char name_11[] PROGMEM = "Rainbow";
const char name_12[] PROGMEM = "Rainbow Cycle";
const char name_13[] PROGMEM = "Scan";
const char name_14[] PROGMEM = "Dual Scan";
const char name_15[] PROGMEM = "Fade";
const char name_16[] PROGMEM = "Theater Chase";
const char name_17[] PROGMEM = "Theater Chase Rainbow";
const char name_18[] PROGMEM = "Running Lights";
const char name_19[] PROGMEM = "Twinkle";
const char name_20[] PROGMEM = "Twinkle Random";
const char name_21[] PROGMEM = "Twinkle Fade";
const char name_22[] PROGMEM = "Twinkle Fade Random";
const char name_23[] PROGMEM = "Sparkle";
const char name_24[] PROGMEM = "Flash Sparkle";
const char name_25[] PROGMEM = "Hyper Sparkle";
const char name_26[] PROGMEM = "Strobe";
const char name_27[] PROGMEM = "Strobe Rainbow";
const char name_28[] PROGMEM = "Multi Strobe";
const char name_29[] PROGMEM = "Blink Rainbow";
const char name_30[] PROGMEM = "Chase White";
const char name_31[] PROGMEM = "Chase Color";
const char name_32[] PROGMEM = "Chase Random";
const char name_33[] PROGMEM = "Chase Rainbow";
const char name_34[] PROGMEM = "Chase Flash";
const char name_35[] PROGMEM = "Chase Flash Random";
const char name_36[] PROGMEM = "Chase Rainbow White";
const char name_37[] PROGMEM = "Chase Blackout";
const char name_38[] PROGMEM = "Chase Blackout Rainbow";
const char name_39[] PROGMEM = "Color Sweep Random";
const char name_40[] PROGMEM = "Running Color";
const char name_41[] PROGMEM = "Running Red Blue";
const char name_42[] PROGMEM = "Running Random";
const char name_43[] PROGMEM = "Larson Scanner";
const char name_44[] PROGMEM = "Comet";
const char name_45[] PROGMEM = "Fireworks";
const char name_46[] PROGMEM = "Fireworks Random";
const char name_47[] PROGMEM = "Merry Christmas";
const char name_48[] PROGMEM = "Fire Flicker";
const char name_49[] PROGMEM = "Fire Flicker (soft)";
const char name_50[] PROGMEM = "Fire Flicker (intense)";
const char name_51[] PROGMEM = "Circus Combustus";
const char name_52[] PROGMEM = "Halloween";
const char name_53[] PROGMEM = "Bicolor Chase";
const char name_54[] PROGMEM = "Tricolor Chase";
const char name_55[] PROGMEM = "Custom 0"; // custom modes need to go at the end
const char name_56[] PROGMEM = "Custom 1";
const char name_57[] PROGMEM = "Custom 2";
const char name_58[] PROGMEM = "Custom 3";
const char name_59[] PROGMEM = "Custom 4";
const char name_60[] PROGMEM = "Custom 5";
const char name_61[] PROGMEM = "Custom 6";
const char name_62[] PROGMEM = "Custom 7";
*/

/*
static const __FlashStringHelper* _names[] =
{
	FSH(name_0),
	FSH(name_1),
	FSH(name_2),
	FSH(name_3),
	FSH(name_4),
	FSH(name_5),
	FSH(name_6),
	FSH(name_7),
	FSH(name_8),
	FSH(name_9),
	FSH(name_10),
	FSH(name_11),
	FSH(name_12),
	FSH(name_13),
	FSH(name_14),
	FSH(name_15),
	FSH(name_16),
	FSH(name_17),
	FSH(name_18),
	FSH(name_19),
	FSH(name_20),
	FSH(name_21),
	FSH(name_22),
	FSH(name_23),
	FSH(name_24),
	FSH(name_25),
	FSH(name_26),
	FSH(name_27),
	FSH(name_28),
	FSH(name_29),
	FSH(name_30),
	FSH(name_31),
	FSH(name_32),
	FSH(name_33),
	FSH(name_34),
	FSH(name_35),
	FSH(name_36),
	FSH(name_37),
	FSH(name_38),
	FSH(name_39),
	FSH(name_40),
	FSH(name_41),
	FSH(name_42),
	FSH(name_43),
	FSH(name_44),
	FSH(name_45),
	FSH(name_46),
	FSH(name_47),
	FSH(name_48),
	FSH(name_49),
	FSH(name_50),
	FSH(name_51),
	FSH(name_52),
	FSH(name_53),
	FSH(name_54),
	FSH(name_55),
	FSH(name_56),
	FSH(name_57),
	FSH(name_58),
	FSH(name_59),
	FSH(name_60),
	FSH(name_61),
	FSH(name_62)
};
*/

static const char* _names[] =
{
"Static",
"Blink",
"Breath",
"Color Wipe",
"Color Wipe Inverse",
"Color Wipe Reverse",
"Color Wipe Reverse Inverse",
"Color Wipe Random",
"Random Color",
"Single Dynamic",
"Multi Dynamic",
"Rainbow",
"Rainbow Cycle",
"Scan",
"Dual Scan",
"Fade",
"Theater Chase",
"Theater Chase Rainbow",
"Running Lights",
"Twinkle",
"Twinkle Random",
"Twinkle Fade",
"Twinkle Fade Random",
"Sparkle",
"Flash Sparkle",
"Hyper Sparkle",
"Strobe",
"Strobe Rainbow",
"Multi Strobe",
"Blink Rainbow",
"Chase White",
"Chase Color",
"Chase Random",
"Chase Rainbow",
"Chase Flash",
"Chase Flash Random",
"Chase Rainbow White",
"Chase Blackout",
"Chase Blackout Rainbow",
"Color Sweep Random",
"Running Color",
"Running Red Blue",
"Running Random",
"Larson Scanner",
"Comet",
"Fireworks",
"Fireworks Random",
"Merry Christmas",
"Fire Flicker",
"Fire Flicker (soft)",
"Fire Flicker (intense)",
"Circus Combustus",
"Halloween",
"Bicolor Chase",
"Tricolor Chase",
"Custom 0", // custom modes need to go at the end
"Custom 1",
"Custom 2",
"Custom 3",
"Custom 4",
"Custom 5",
"Custom 6",
"Custom 7",
};

// class WS2812FX : public Adafruit_NeoPixel
// {

// public:
	typedef uint16_t (*WS2812FX_mode_ptr)(void);

	// segment parameters
	typedef struct Segment   // 20 bytes
	{
		uint16_t start;
		uint16_t stop;
		uint16_t speed;
		uint8_t  mode;
		uint8_t  options;
		uint32_t colors[MAX_NUM_COLORS];
	} segment;

	// segment runtime parameters
	typedef struct Segment_runtime   // 16 bytes
	{
		unsigned long next_time;
		uint32_t counter_mode_step;
		uint32_t counter_mode_call;
		uint8_t aux_param;   // auxilary param (usually stores a color_wheel index)
		uint8_t aux_param2;  // auxilary param (usually stores bitwise options)
		uint16_t aux_param3; // auxilary param (usually stores a segment index)
	} segment_runtime;

	// ????????????
	void WS2812FX_Init(void);
/*
	WS2812FX_Init(uint16_t num_leds, uint8_t pin, neoPixelType type,
			 uint8_t max_num_segments = MAX_NUM_SEGMENTS,
			 uint8_t max_num_active_segments = MAX_NUM_ACTIVE_SEGMENTS)
		: Adafruit_NeoPixel(num_leds, pin, type)
	{

		brightness = DEFAULT_BRIGHTNESS + 1; // Adafruit_NeoPixel internally offsets brightness by 1
		_running = false;

		_segments_len = max_num_segments;
		_active_segments_len = max_num_active_segments;

		// create all the segment arrays and init to zeros
		_segments = new segment[_segments_len]();
		_active_segments = new uint8_t[_active_segments_len]();
		_segment_runtimes = new segment_runtime[_active_segments_len]();

		// init segment pointers
		_seg     = _segments;
		_seg_rt  = _segment_runtimes;

		resetSegments();
		setSegment(0, 0, num_leds - 1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
	}
*/

	void
		// timer(void),
		WS2812FX_service(void),
		WS2812FX_init(void),
		WS2812FX_start(void),
		WS2812FX_stop(void),
		WS2812FX_pause(void),
		WS2812FX_resume(void),
		WS2812FX_strip_off(void),
		WS2812FX_fade_out_void(void),
		WS2812FX_fade_out_u32(uint32_t),
		WS2812FX_setMode_param1(uint8_t m),
		WS2812FX_setMode_param2(uint8_t seg, uint8_t m),
		WS2812FX_setOptions(uint8_t seg, uint8_t o),
		WS2812FX_setCustomMode(uint16_t (*p)()),
		WS2812FX_setCustomShow(void (*p)()),
		WS2812FX_setSpeed_param1(uint16_t s),
		WS2812FX_setSpeed_param2(uint8_t seg, uint16_t s),
		WS2812FX_increaseSpeed(uint8_t s),
		WS2812FX_decreaseSpeed(uint8_t s),
		WS2812FX_setColor_rgb(uint8_t r, uint8_t g, uint8_t b),
		WS2812FX_setColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
		WS2812FX_setColor_u32(uint32_t c),
		WS2812FX_setColor_seg(uint8_t seg, uint32_t c),
		WS2812FX_setColors(uint8_t seg, uint32_t* c),
		WS2812FX_setBrightness(uint8_t b),
		WS2812FX_increaseBrightness(uint8_t s),
		WS2812FX_decreaseBrightness(uint8_t s),

		// ??????????
		// WS2812FX_setLength(uint16_t b),
		// WS2812FX_increaseLength(uint16_t s),
		// WS2812FX_decreaseLength(uint16_t s),

		WS2812FX_trigger(void),
		WS2812FX_setCycle(void),
		WS2812FX_setNumSegments(uint8_t n),
		WS2812FX_setSegment_color_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, bool reverse),
		WS2812FX_setSegment_color_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
		WS2812FX_setSegment_colors_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse),
		WS2812FX_setSegment_colors_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
		WS2812FX_setIdleSegment_color_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
		WS2812FX_setIdleSegment_colors_opt(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
		WS2812FX_addActiveSegment(uint8_t seg),
		WS2812FX_removeActiveSegment(uint8_t seg),
		WS2812FX_swapActiveSegment(uint8_t oldSeg, uint8_t newSeg),
		WS2812FX_resetSegments(void),
		WS2812FX_resetSegmentRuntimes(void),
		WS2812FX_resetSegmentRuntime(uint8_t),
		WS2812FX_setPixelColor_u32(uint16_t n, uint32_t c),
		WS2812FX_setPixelColor_rgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
		WS2812FX_setPixelColor_rgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
		WS2812FX_copyPixels(uint16_t d, uint16_t s, uint16_t c),
		WS2812FX_setPixels(uint16_t, uint8_t*),
		WS2812FX_show(void);

	bool
		WS2812FX_isRunning(void),
		WS2812FX_isTriggered(void),
		WS2812FX_isFrame_void(void),
		WS2812FX_isFrame_u8(uint8_t),
		WS2812FX_isCycle_void(void),
		WS2812FX_isCycle_u8(uint8_t),
		WS2812FX_isActiveSegment(uint8_t seg);

	uint8_t
		WS2812FX_random8_void(void),
		WS2812FX_random8_u8(uint8_t),
		WS2812FX_getMode_void(void),
		WS2812FX_getMode_u8(uint8_t),
		WS2812FX_getModeCount(void),
		WS2812FX_setCustomMode_param2(const char* name, uint16_t (*p)()),
		WS2812FX_setCustomMode_param3(uint8_t i, const char* name, uint16_t (*p)()),
		WS2812FX_getNumSegments(void),
		WS2812FX_get_random_wheel_index(uint8_t),
		WS2812FX_getOptions(uint8_t),
		WS2812FX_getNumBytesPerPixel(void);

	uint16_t
		WS2812FX_random16_void(void),
		WS2812FX_random16_u16(uint16_t),
		WS2812FX_getSpeed_void(void),
		WS2812FX_getSpeed_u8(uint8_t),
		WS2812FX_getLength(void),
		WS2812FX_getNumBytes(void);

	uint32_t
		WS2812FX_color_wheel(uint8_t),
		WS2812FX_getColor_void(void),
		WS2812FX_getColor_u8(uint8_t),
		WS2812FX_intensitySum(void);

	uint32_t* WS2812FX_getColors(uint8_t);
	uint32_t* WS2812FX_intensitySums(void);
	uint8_t*  WS2812FX_getActiveSegments(void);

	const char* WS2812FX_getModeName(uint8_t m);

	segment* WS2812FX_getSegment_void(void);

	segment* WS2812FX_getSegment_u8(uint8_t);

	segment* WS2812FX_getSegments(void);

	segment_runtime* WS2812FX_getSegmentRuntime_void(void);

	segment_runtime* WS2812FX_getSegmentRuntime_u8(uint8_t);

	segment_runtime* WS2812FX_getSegmentRuntimes(void);

	// mode helper functions
	uint16_t
		WS2812FX_blink(uint32_t, uint32_t, bool strobe),
		WS2812FX_color_wipe(uint32_t, uint32_t, bool),
		WS2812FX_twinkle(uint32_t, uint32_t),
		WS2812FX_twinkle_fade(uint32_t),
		WS2812FX_sparkle(uint32_t, uint32_t),
		WS2812FX_chase(uint32_t, uint32_t, uint32_t),
		WS2812FX_chase_flash(uint32_t, uint32_t),
		WS2812FX_running(uint32_t, uint32_t),
		WS2812FX_fireworks(uint32_t),
		WS2812FX_fire_flicker(int),
		WS2812FX_tricolor_chase(uint32_t, uint32_t, uint32_t),
		WS2812FX_scan(uint32_t, uint32_t, bool);

	uint32_t
	WS2812FX_color_blend(uint32_t, uint32_t, uint8_t);

	// builtin modes
	uint16_t
		WS2812FX_mode_blink_rainbow(void),
		WS2812FX_mode_blink(void),
		WS2812FX_mode_static(void),
		WS2812FX_mode_strobe(void),
		WS2812FX_mode_strobe_rainbow(void),
		WS2812FX_mode_color_wipe(void),
		WS2812FX_mode_color_wipe_inv(void),
		WS2812FX_mode_color_wipe_rev(void),
		WS2812FX_mode_color_wipe_rev_inv(void),
		WS2812FX_mode_color_wipe_random(void),
		WS2812FX_mode_color_sweep_random(void),
		WS2812FX_mode_random_color(void),
		WS2812FX_mode_single_dynamic(void),
		WS2812FX_mode_multi_dynamic(void),
		WS2812FX_mode_breath(void),
		WS2812FX_mode_fade(void),
		WS2812FX_mode_scan(void),
		WS2812FX_mode_dual_scan(void),
		WS2812FX_mode_theater_chase(void),
		WS2812FX_mode_theater_chase_rainbow(void),
		WS2812FX_mode_rainbow(void),
		WS2812FX_mode_rainbow_cycle(void),
		WS2812FX_mode_running_lights(void),
		WS2812FX_mode_twinkle(void),
		WS2812FX_mode_twinkle_random(void),
		WS2812FX_mode_twinkle_fade(void),
		WS2812FX_mode_twinkle_fade_random(void),
		WS2812FX_mode_sparkle(void),
		WS2812FX_mode_flash_sparkle(void),
		WS2812FX_mode_hyper_sparkle(void),
		WS2812FX_mode_multi_strobe(void),
		WS2812FX_mode_chase_white(void),
		WS2812FX_mode_chase_color(void),
		WS2812FX_mode_chase_random(void),
		WS2812FX_mode_chase_rainbow(void),
		WS2812FX_mode_chase_flash(void),
		WS2812FX_mode_chase_flash_random(void),
		WS2812FX_mode_chase_rainbow_white(void),
		WS2812FX_mode_chase_blackout(void),
		WS2812FX_mode_chase_blackout_rainbow(void),
		WS2812FX_mode_running_color(void),
		WS2812FX_mode_running_red_blue(void),
		WS2812FX_mode_running_random(void),
		WS2812FX_mode_larson_scanner(void),
		WS2812FX_mode_comet(void),
		WS2812FX_mode_fireworks(void),
		WS2812FX_mode_fireworks_random(void),
		WS2812FX_mode_merry_christmas(void),
		WS2812FX_mode_halloween(void),
		WS2812FX_mode_fire_flicker(void),
		WS2812FX_mode_fire_flicker_soft(void),
		WS2812FX_mode_fire_flicker_intense(void),
		WS2812FX_mode_circus_combustus(void),
		WS2812FX_mode_bicolor_chase(void),
		WS2812FX_mode_tricolor_chase(void);
		// WS2812FX_mode_custom_0(void),
		// WS2812FX_mode_custom_1(void),
		// WS2812FX_mode_custom_2(void),
		// WS2812FX_mode_custom_3(void),
		// WS2812FX_mode_custom_4(void),
		// WS2812FX_mode_custom_5(void),
		// WS2812FX_mode_custom_6(void),
		// WS2812FX_mode_custom_7(void);

// private:
	// uint16_t _rand16seed;

// ??????????????????????????????????????????????????
#if 0
	uint16_t (*customModes[MAX_CUSTOM_MODES])(void)
	{
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; },
		[] { return (uint16_t)1000; }
	};
	void (*customShow)(void) = NULL;
#endif

	// bool
	// 	_running,
	// 	_triggered;

	// segment* _segments;                 // array of segments (20 bytes per element)
	// segment_runtime* _segment_runtimes; // array of segment runtimes (16 bytes per element)
	// uint8_t* _active_segments;          // array of active segments (1 bytes per element)

	// uint8_t _segments_len = 0;          // size of _segments array
	// uint8_t _active_segments_len = 0;   // size of _segments_runtime and _active_segments arrays
	// uint8_t _num_segments = 0;          // number of configured segments in the _segments array

	// uint8_t _segments_len;          // size of _segments array
	// uint8_t _active_segments_len;   // size of _segments_runtime and _active_segments arrays
	// uint8_t _num_segments;          // number of configured segments in the _segments array

	// segment* _seg;                      // currently active segment (20 bytes)
	// segment_runtime* _seg_rt;           // currently active segment runtime (16 bytes)

	// uint16_t _seg_len;                  // num LEDs in the currently active segment
// };

// define static array of member function pointers.
// function pointers MUST be in the same order as the corresponding name in the _name array.
// static uint16_t (*_modes[MODE_COUNT])(void);
static WS2812FX_mode_ptr _modes[MODE_COUNT] =
{
	WS2812FX_mode_static,
	WS2812FX_mode_blink,
	WS2812FX_mode_breath,
	WS2812FX_mode_color_wipe,
	WS2812FX_mode_color_wipe_inv,
	WS2812FX_mode_color_wipe_rev,
	WS2812FX_mode_color_wipe_rev_inv,
	WS2812FX_mode_color_wipe_random,
	WS2812FX_mode_random_color,
	WS2812FX_mode_single_dynamic,
	WS2812FX_mode_multi_dynamic,
	WS2812FX_mode_rainbow,
	WS2812FX_mode_rainbow_cycle,
	WS2812FX_mode_scan,
	WS2812FX_mode_dual_scan,
	WS2812FX_mode_fade,
	WS2812FX_mode_theater_chase,
	WS2812FX_mode_theater_chase_rainbow,
	WS2812FX_mode_running_lights,
	WS2812FX_mode_twinkle,
	WS2812FX_mode_twinkle_random,
	WS2812FX_mode_twinkle_fade,
	WS2812FX_mode_twinkle_fade_random,
	WS2812FX_mode_sparkle,
	WS2812FX_mode_flash_sparkle,
	WS2812FX_mode_hyper_sparkle,
	WS2812FX_mode_strobe,
	WS2812FX_mode_strobe_rainbow,
	WS2812FX_mode_multi_strobe,
	WS2812FX_mode_blink_rainbow,
	WS2812FX_mode_chase_white,
	WS2812FX_mode_chase_color,
	WS2812FX_mode_chase_random,
	WS2812FX_mode_chase_rainbow,
	WS2812FX_mode_chase_flash,
	WS2812FX_mode_chase_flash_random,
	WS2812FX_mode_chase_rainbow_white,
	WS2812FX_mode_chase_blackout,
	WS2812FX_mode_chase_blackout_rainbow,
	WS2812FX_mode_color_sweep_random,
	WS2812FX_mode_running_color,
	WS2812FX_mode_running_red_blue,
	WS2812FX_mode_running_random,
	WS2812FX_mode_larson_scanner,
	WS2812FX_mode_comet,
	WS2812FX_mode_fireworks,
	WS2812FX_mode_fireworks_random,
	WS2812FX_mode_merry_christmas,
	WS2812FX_mode_fire_flicker,
	WS2812FX_mode_fire_flicker_soft,
	WS2812FX_mode_fire_flicker_intense,
	WS2812FX_mode_circus_combustus,
	WS2812FX_mode_halloween,
	WS2812FX_mode_bicolor_chase,
	WS2812FX_mode_tricolor_chase,
	// WS2812FX_mode_custom_0,
	// WS2812FX_mode_custom_1,
	// WS2812FX_mode_custom_2,
	// WS2812FX_mode_custom_3,
	// WS2812FX_mode_custom_4,
	// WS2812FX_mode_custom_5,
	// WS2812FX_mode_custom_6,
	// WS2812FX_mode_custom_7,
};
#endif
