#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tos_k.h"
#include "stm32g0xx_ll_tim.h"
#include "user_task.h"
#include "user_hw_init.h"
#include "encoder.h"
#include "WS2812FX.h"
#include "oled.h"
#include "gui.h"
#include "font_lite.h"
#include "thumbs_up.h"
#include "at_client.h"


// 配网提示图片，文字内容及格式如下
// 打开微信小程序腾讯连
// 连，点击添加设备=>灯
// 128 * 32
const uint8_t wifi_config_pic1[] =
{
	0x00,0x00,0x00,0x00,0x20,0x20,0xFC,0x20,0x20,0x08,0x08,0x08,0xF8,0x08,0x08,0x00,
	0x00,0x08,0x08,0xF8,0x08,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x90,0xC8,0x24,0x38,
	0xA0,0xBC,0xA0,0x78,0xA0,0x1C,0xF0,0x00,0x40,0xF0,0x0C,0x10,0x50,0x50,0x54,0x58,
	0x50,0x50,0x10,0x00,0x00,0x00,0xC0,0x00,0x00,0xFC,0x00,0x00,0x40,0x80,0x00,0x00,
	0x48,0x48,0xF8,0x44,0x00,0xBC,0xA4,0xA4,0xA4,0xBC,0x00,0x00,0x00,0xF8,0x08,0x28,
	0xA8,0xA8,0x2C,0xA8,0x68,0x28,0x08,0x00,0x00,0xFC,0x24,0xFC,0x40,0xD4,0x70,0x5C,
	0x50,0xD4,0x40,0x00,0x40,0x44,0xC8,0x00,0x88,0xF8,0x88,0x88,0xF8,0x00,0x00,0x00,
	0x84,0x88,0x00,0x10,0xD0,0xB0,0x9C,0xD0,0x90,0x90,0x10,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x02,0x22,0x3F,0x01,0x01,0x00,0x20,0x20,0x3F,0x00,0x00,0x00,
	0x01,0x21,0x19,0x07,0x01,0x01,0x01,0x3F,0x01,0x01,0x01,0x00,0x00,0x3F,0x00,0x20,
	0x1E,0x02,0x1E,0x28,0x13,0x0C,0x33,0x00,0x00,0x3F,0x00,0x00,0x3D,0x15,0x15,0x15,
	0x15,0x3D,0x00,0x00,0x04,0x03,0x00,0x20,0x20,0x3F,0x00,0x00,0x00,0x01,0x06,0x00,
	0x04,0x03,0x3F,0x01,0x22,0x24,0x24,0x3F,0x24,0x24,0x20,0x00,0x20,0x1F,0x02,0x02,
	0x02,0x22,0x3F,0x02,0x02,0x0A,0x06,0x00,0x20,0x1F,0x21,0x3F,0x11,0x17,0x15,0x15,
	0x17,0x24,0x3D,0x00,0x00,0x00,0x1F,0x08,0x00,0x3F,0x00,0x00,0x0F,0x10,0x38,0x00,
	0x20,0x1F,0x20,0x24,0x24,0x24,0x24,0x3F,0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x84,0x88,0x00,0x10,0xD0,0xB0,0x9C,0xD0,0x90,0x90,0x10,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x40,0x40,
	0x40,0x7C,0x48,0x48,0x48,0xC8,0x08,0x00,0x80,0x90,0x90,0x90,0x90,0xFC,0x90,0x90,
	0x90,0x90,0x80,0x00,0x88,0x10,0x00,0x20,0xA4,0x64,0x3C,0x24,0x64,0xA4,0x20,0x00,
	0x20,0x20,0xFC,0x20,0x20,0xE0,0x00,0xF0,0x10,0x10,0xF0,0x00,0x40,0x44,0xC8,0x00,
	0x40,0x3C,0x04,0x04,0x3C,0x40,0x40,0x00,0x80,0x90,0x48,0x5C,0x28,0x28,0x28,0x58,
	0x48,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0xF0,0xE0,0xC0,0x80,0x00,0x00,
	0x00,0xE0,0x00,0xFC,0x40,0x20,0x08,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x20,0x1F,0x20,0x24,0x24,0x24,0x24,0x3F,0x24,0x24,0x24,0x00,
	0x00,0x00,0x0B,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x17,0x04,0x14,
	0x24,0x04,0x14,0x24,0x04,0x17,0x20,0x00,0x00,0x1E,0x10,0x10,0x10,0x1F,0x10,0x10,
	0x10,0x3E,0x00,0x00,0x10,0x09,0x04,0x11,0x08,0x20,0x3F,0x04,0x18,0x04,0x19,0x00,
	0x20,0x18,0x07,0x20,0x20,0x1F,0x00,0x3F,0x10,0x10,0x3F,0x00,0x00,0x00,0x1F,0x08,
	0x21,0x23,0x15,0x09,0x15,0x23,0x20,0x00,0x00,0x00,0x3F,0x15,0x15,0x1F,0x15,0x15,
	0x3F,0x00,0x00,0x00,0x00,0x03,0x03,0x03,0x03,0x03,0x1F,0x0F,0x07,0x03,0x01,0x00,
	0x00,0x20,0x18,0x07,0x08,0x30,0x00,0x20,0x20,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,
};

// 配网提示图片，文字内容及格式如下
// 当手机提示添加完成后
// 请重启设备
// 128 * 32
const uint8_t wifi_config_pic2[] =
{
	0x00,0x00,0x00,0x00,0x00,0x48,0x50,0x40,0x40,0x7C,0x40,0x40,0x50,0xC8,0x00,0x00,
	0x00,0x48,0x48,0x48,0x48,0xF8,0x44,0x44,0x44,0x44,0x00,0x00,0x20,0xA0,0xFC,0xA0,
	0x00,0xF8,0x08,0x08,0xF8,0x00,0x00,0x00,0x20,0x20,0xFC,0x20,0x20,0x7C,0x54,0x54,
	0x54,0x7C,0x00,0x00,0x40,0x40,0x48,0x48,0x48,0xC8,0x48,0x48,0x48,0x40,0x40,0x00,
	0x88,0x10,0x00,0x20,0xA4,0x64,0x3C,0x24,0x64,0xA4,0x20,0x00,0x20,0x20,0xFC,0x20,
	0x20,0xE0,0x00,0xF0,0x10,0x10,0xF0,0x00,0x18,0x08,0x48,0x48,0x48,0x4C,0x48,0x48,
	0x48,0x08,0x18,0x00,0x00,0xF0,0x90,0x90,0x90,0x10,0xFC,0x10,0x14,0x98,0x10,0x00,
	0x00,0x00,0xF8,0x48,0x48,0x48,0x48,0x44,0x44,0x44,0x40,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x10,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x3F,0x00,0x00,
	0x02,0x02,0x02,0x22,0x22,0x3F,0x02,0x02,0x02,0x02,0x02,0x00,0x02,0x01,0x3F,0x20,
	0x11,0x0F,0x00,0x00,0x1F,0x20,0x38,0x00,0x02,0x22,0x3F,0x01,0x20,0x1D,0x21,0x3F,
	0x25,0x25,0x21,0x00,0x10,0x08,0x06,0x00,0x20,0x3F,0x00,0x00,0x02,0x04,0x18,0x00,
	0x10,0x09,0x04,0x11,0x08,0x20,0x3F,0x04,0x18,0x04,0x19,0x00,0x20,0x18,0x07,0x20,
	0x20,0x1F,0x00,0x3F,0x10,0x10,0x3F,0x00,0x22,0x22,0x12,0x0E,0x02,0x02,0x02,0x1E,
	0x22,0x22,0x32,0x00,0x20,0x1F,0x00,0x08,0x2F,0x10,0x0B,0x04,0x0A,0x11,0x3C,0x00,
	0x20,0x18,0x07,0x00,0x3E,0x12,0x12,0x12,0x12,0x12,0x3E,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x44,0xC8,0x00,0x88,0xA8,0xA8,0xFC,0xA8,0xA8,0xA8,0x88,0x00,
	0x20,0xE8,0xA8,0xA8,0xA8,0xF8,0xA4,0xA4,0xA4,0xE4,0x20,0x00,0x00,0x00,0xF0,0x90,
	0x90,0x94,0x98,0x90,0x90,0x90,0xF0,0x00,0x40,0x44,0xC8,0x00,0x40,0x3C,0x04,0x04,
	0x3C,0x40,0x40,0x00,0x80,0x90,0x48,0x5C,0x28,0x28,0x28,0x58,0x48,0x80,0x80,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x3F,0x10,0x00,0x3F,0x0A,0x0A,0x0A,0x2A,0x3F,0x00,0x00,
	0x20,0x2B,0x2A,0x2A,0x2A,0x3F,0x2A,0x2A,0x2A,0x2B,0x20,0x00,0x20,0x18,0x07,0x3E,
	0x12,0x12,0x12,0x12,0x12,0x12,0x3E,0x00,0x00,0x00,0x1F,0x08,0x21,0x23,0x15,0x09,
	0x15,0x23,0x20,0x00,0x00,0x00,0x3F,0x15,0x15,0x1F,0x15,0x15,0x3F,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

// 14 * 11
const uint8_t wifi_logo[] =
{
	0x08,0x04,0x22,0x12,0x91,0x49,0x49,0x49,
	0x49,0x91,0x12,0x22,0x04,0x08,0x00,0x00,
	0x00,0x00,0x00,0x00,0x06,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,
};

// 16 * 11
const uint8_t cloud_logo[] =
{
	0xC0,0x20,0x10,0x1C,0x22,0x41,0x01,0x01,
	0x81,0x41,0x22,0x14,0x10,0x10,0x20,0xC0,
	0x01,0x02,0x04,0x04,0x04,0x04,0x06,0x05,
	0x04,0x04,0x04,0x04,0x04,0x04,0x02,0x01,
};

// 27 * 28
const uint8_t light_on[] =
{
	0x00,0x00,0x00,0x08,0x10,0x20,0x00,0x00,0x80,0x40,0x40,0x20,0xA0,0xAF,0xA0,0x20,
	0x40,0x40,0x80,0x00,0x00,0x20,0x10,0x08,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x00,
	0xF8,0x06,0x71,0xFC,0xFE,0xFF,0xFF,0xFF,0xFD,0xFD,0xFB,0xF7,0xCE,0xFC,0x71,0x06,
	0xF8,0x00,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x80,0x40,0x20,0x03,0x04,0x09,0xF3,
	0xA7,0xAF,0xAF,0xAF,0xAF,0xAF,0xA7,0xF3,0x09,0x04,0x03,0x10,0x20,0x40,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x0E,0x0A,0x0A,0x0A,
	0x0E,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

// 27 * 28
const uint8_t light_off[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x40,0x20,0x20,0x20,0x20,0x20,
	0x40,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xF8,0x06,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,
	0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x04,0x08,0xF0,
	0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xF0,0x08,0x04,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x0E,0x0A,0x0A,0x0A,
	0x0E,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


// 色温设置范围
#define COLOR_TEMP_MAX 6500
#define COLOR_TEMP_MIN 2700
#define COLOR_TEMP_MID ((COLOR_TEMP_MAX + COLOR_TEMP_MIN) / 2)

// 七彩模式数
#define MAX_COLORFUL_MODE_NUM 55

// 屏幕内最多可显示的菜单条目数
#define MENU_MAX_SHOW_NUM 2

// 滚动条相关
// #define SCROLL_BAR_MAX_LENGTH 28
#define SCROLL_BAR_MAX_LENGTH (LCD_WIDTH - 5) // 滚动条上下的边框各占2像素，因为运算原因再多加1像素


typedef struct _MenuNode{
	uint8_t num; // _MenuItem 中菜单条目总数
	uint8_t cursor; // 菜单光标位置
	uint8_t head; // 菜单显示的第一条
	uint8_t fill[1]; // 填充，字节对齐
	struct _MenuNode *parent; // 父菜单
	struct _MenuItem *item; // 菜单项
}MenuNode;

typedef struct _MenuItem{
	char *ItemName; // 本项菜单的名字
	void (*function)(void); // 功能函数
	struct _MenuNode *child; // 子菜单节点
}MenuItem;

MenuNode MenuNode0;

void wifi_config(void);
void play_pixel_images(void);
void colorful_menu(void);

MenuItem MenuItem0[] =
{
	{"返回", NULL, NULL},
	{"彩灯模式", &colorful_menu, NULL},
	{"配网", &wifi_config, NULL},
	{"一键三连", &play_pixel_images, NULL},
};

MenuNode MenuNode0 = 
{
	.num = sizeof(MenuItem0) / sizeof(MenuItem),
	.cursor = 0,
	.head = 0,
	.parent = NULL,
	.item = &MenuItem0[0],
};


char menu_line1[24] = {0};
char menu_line2[24] = {0};

MenuNode *menu_p = NULL;
uint8_t menu_cursor_pos = 0;
uint8_t menu_show_pos = 0;

// 彩灯模式与彩灯默认颜色
// static uint8_t default_color_mode = 0; // 0随机，1自定义
static uint8_t colorful_mode = 0;
// static uint32_t default_color = 0;

// 有关灯光设置的相关变量，仅供 set_brightness(),set_color_temp() 计算亮度和色温使用
static uint32_t brightness_yellow = 0;
static uint32_t brightness_white = 0;
static uint32_t brightness_yellow_out = 0;
static uint32_t brightness_white_out = 0;

// 灯光亮度及色温，全局控制
static uint8_t power_sw = 0; // 仅通过 power_switch() 设置
static int32_t color_temp_value = 5500; // 仅通过set_color_temp() 设置
static int32_t brightness_value = 0; // 仅通过 set_brightness() 设置

// 屏幕控制相关
static uint8_t oled_power_state = 1;
static uint32_t oled_timer = 0;

void menu_show_select_item(MenuNode *menu_p)
{
	// c = (a+(b/2))/b;

	uint16_t bar_len = SCROLL_BAR_MAX_LENGTH * MENU_MAX_SHOW_NUM / (menu_p->num + 1);
	// uint16_t scroll_unit = (SCROLL_BAR_MAX_LENGTH - bar_len) / (menu_p->num - 1);
	uint16_t scroll_start = 0;
	uint16_t scroll_end = 0;
	// uint16_t scroll_start = menu_p->cursor * (SCROLL_BAR_MAX_LENGTH - bar_len) / (menu_p->num - 1) + 2;
	// uint16_t scroll_end = menu_p->cursor * (SCROLL_BAR_MAX_LENGTH - bar_len) / (menu_p->num - 1) + bar_len + 2;

	if(menu_p->cursor == menu_p->head)
	{
		menu_line1[0] = '>';
		menu_line2[0] = ' ';
	}
	else
	{
		menu_line1[0] = ' ';
		menu_line2[0] = '>';
	}
	
	sprintf(&menu_line1[1], "%s\n", menu_p->item[menu_p->head].ItemName);
	sprintf(&menu_line2[1], "%s\n", menu_p->item[menu_p->head + 1].ItemName);

	CleanScreen();

	// 菜单滚动条
	DrawLine(123, 0, 126, 0, 1);
	DrawLine(123, 31, 126, 31, 1);
	DrawLine(122, 1, 122, 30, 1);
	DrawLine(127, 1, 127, 30, 1);
	if(menu_p->num != 1)
	{
		scroll_start = menu_p->cursor * (SCROLL_BAR_MAX_LENGTH - bar_len) / (menu_p->num - 1) + 2;
		scroll_end = menu_p->cursor * (SCROLL_BAR_MAX_LENGTH - bar_len) / (menu_p->num - 1) + bar_len + 2;
		DrawRectangle(124, scroll_start, 124, scroll_end, 1);
		DrawRectangle(125, scroll_start, 125, scroll_end, 1);
	}
	else
	{
		DrawRectangle(124, 2, 124, LCD_WIDTH - 2, 1);
		DrawRectangle(125, 2, 125, LCD_WIDTH - 2, 1);
	}

	PrintString12(0, 0, menu_line1);
	PrintString12(0, 14, menu_line2);
	RefreshFullScreen();
}

void main_screen(void)
{
	static uint8_t key_state = 0; // 按下并旋转调整色温后，不进入菜单
	
	uint8_t brightness_change = 0;
	uint8_t key = 0;
	int8_t encoder = 0;
	int8_t encoder_fix = 0;

	uint8_t power_sw_tmp = power_sw;
	int32_t brightness_value_tmp = brightness_value;
	int32_t color_temp_value_tmp = color_temp_value;

	uint32_t connection_state = 0;


	encoder = ReadEncoder();
	key = ReadEncoderKey();

	encoder_fix = encoder * abs(encoder); // 实现转速越快，数值增长越快的效果

	// 旋转调整亮度
	if(key == ENCODER_KEY_NULL)
	{
		if(encoder)
		{
			brightness_change = 1;
		}
		if (brightness_value_tmp + encoder_fix < 0)
		{
			brightness_value_tmp = 0;
		}
		else if (brightness_value_tmp + encoder_fix > 100)
		{
			brightness_value_tmp = 100;
		}
		else
		{
			brightness_value_tmp += encoder_fix;
		}
	}

	// 按下并旋转调整色温
	if(key == ENCODER_KEY_PRESS)
	{
		if(encoder_fix)
		{
			key_state = 1;
		}

		if (color_temp_value_tmp + encoder_fix * 50 < COLOR_TEMP_MIN)
		{
			color_temp_value_tmp = COLOR_TEMP_MIN;
		}
		else if (color_temp_value_tmp + encoder_fix * 50 > COLOR_TEMP_MAX)
		{
			color_temp_value_tmp = COLOR_TEMP_MAX;
		}
		else
		{
			color_temp_value_tmp += encoder_fix * 50;
		}
	}

	set_brightness(brightness_value_tmp);
	set_color_temp(color_temp_value_tmp);
	
	if(brightness_value_tmp == 0)
	{
		power_sw_tmp = 0;
	}
	if(brightness_change && brightness_value_tmp)
	{
		power_sw_tmp = 1;
	}
	power_switch(power_sw_tmp);

	if(key == ENCODER_KEY_RELEASE)
	{
		if(key_state)
		{
			key_state = 0;
		}
		else
		{
			menu_p = &MenuNode0;
		}
	}

	CleanScreen();

	if(power_sw)
	{
		DrawPic(80, 0, 27, 28, (uint8_t *)light_on);
	}
	else
	{
		DrawPic(80, 0, 27, 28, (uint8_t *)light_off);
	}

	connection_state = getFlag();
	if(connection_state & WIFI_CON_FLAG)
	{
		DrawPic(113, 2, 14, 11, (uint8_t *)wifi_logo);
	}
	if(connection_state & MQTT_CON_FLAG)
	{
		DrawPic(112, 18, 16, 11, (uint8_t *)cloud_logo);
	}

	sprintf(&menu_line1[0], "亮 度  %d%%", brightness_value);
	sprintf(&menu_line2[0], "色 温  %dK", color_temp_value);
	PrintString12(0, 0, menu_line1);
	PrintString12(0, 14, menu_line2);
	RefreshFullScreen();
}

void menu_task(void)
{
	uint8_t key = 0;
	int8_t encoder = 0;

	if(oled_power_state == 0)
	{
		// 息屏状态下任意操作唤醒屏幕
		if(ReadEncoder() | ReadEncoderKey())
		{
			oled_power_state = 1;
			OLED_Display_On();
			oled_timer = 0;
		}
	}
	else
	{
		// 一分钟左右熄灭
		if(oled_timer > 900)
		{
			oled_power_state = 0;
			OLED_Display_Off();
		}
		else
		{
			oled_timer += 1;
		}
	}
	

	if(menu_p == NULL)
	{
		main_screen();
	}
	else
	{
		encoder = ReadEncoder();
		key = ReadEncoderKey();

		if(encoder < 0)
		{
			if(menu_p->cursor > 0)
			{
				menu_p->cursor -= 1;
			}
			if(menu_p->head > menu_p->cursor)
			{
				menu_p->head = menu_p->cursor;
			}
		}
		if(encoder > 0)
		{
			if(menu_p->cursor < menu_p->num - 1)
			{
				menu_p->cursor += 1;
			}
			if(menu_p->head + MENU_MAX_SHOW_NUM - 1 < menu_p->cursor)
			{
				menu_p->head = menu_p->cursor - (MENU_MAX_SHOW_NUM - 1);
			}
		}
		if(key == ENCODER_KEY_RELEASE)
		{
			if(menu_p->item[menu_p->cursor].function != NULL)
			{
				menu_p->item[menu_p->cursor].function();
			}
			else if(menu_p->item[menu_p->cursor].child != NULL)
			{
				menu_p = menu_p->item[menu_p->cursor].child;
			}
			else
			{
				menu_p = menu_p->parent;
				if(menu_p == NULL)
				{
					// 直接返回待机界面
					return;
				}
			}
		}
		menu_show_select_item(menu_p);
	}

	return;
}

void play_pixel_images(void)
{
	int32_t i = 0;
	int32_t pos = 0;
	k_task_t *hanndle;

	// __disable_irq();
	hanndle = tos_task_curr_task_get();
	tos_task_prio_change(hanndle, 1);
	for (i = 0; i < 60; i++)
	{
		OLED_DrawBMP(0, 0, 128, 8, (uint8_t *)&thumbs_up[pos]);
		pos += 512;
		tos_sleep_ms(50);
		// delay_us(50000);
	}
	tos_task_prio_change(hanndle, 4);
	// __enable_irq();

	tos_sleep_ms(2000);
}

// 随机模式不支持设定颜色
void set_colorful_mode(uint8_t mode, uint32_t default_color_mode, uint32_t color)
{
	if(mode == 0)
	{
		WS2812FX_stop();
	}
	else
	{
		if(default_color_mode == 0)
		{
			WS2812FX_setColor_u32(WS2812FX_color_wheel(HAL_GetTick() & 0xFF)); // 设置一个随机的初始颜色
		}
		else
		{
			WS2812FX_setColor_u32(color);
		}
		WS2812FX_start();
		WS2812FX_setMode_param1(mode - 1);
	}
}

void colorful_menu(void)
{
	uint8_t key = 0;
	int8_t encoder = 0;

	int8_t encoder_fix = 0;

	while (1)
	{
		encoder = ReadEncoder();
		key = ReadEncoderKey();

		encoder_fix = encoder * abs(encoder); // 实现转速越快，数值增长越快的效果
		if (colorful_mode + encoder_fix < 0)
		{
			colorful_mode = 0;
		}
		else if (colorful_mode + encoder_fix > MAX_COLORFUL_MODE_NUM)
		{
			colorful_mode = MAX_COLORFUL_MODE_NUM;
		}
		else
		{
			colorful_mode += encoder_fix;
		}

		if (key == ENCODER_KEY_RELEASE)
		{
			break;
		}
		
		if(colorful_mode == 0)
		{
			sprintf(&menu_line2[0], "    < 关  闭 >\n");
		}
		else
		{
			sprintf(&menu_line2[0], "    < 模式%02d >\n", colorful_mode);
		}
		set_colorful_mode(colorful_mode, 0, 0);
		
		sprintf(&menu_line1[0], "七彩模式设置\n");

		CleanScreen();
		PrintString12(0, 0, menu_line1);
		PrintString12(0, 14, menu_line2);
		RefreshFullScreen();

		// tos_task_delay(tos_millisec2tick(31));
		tos_sleep_ms(13);
	}
}

void wifi_config(void)
{
	int i;
	// int result = QCLOUD_RET_SUCCESS;
	at_response_t resp = NULL;
	uint8_t key = 0;

	CleanScreen();
	DrawPic(0, 0, 128, 32, (uint8_t *)wifi_config_pic1);
	RefreshFullScreen();

	resp = at_create_resp(64, 0, 2000);

	// TODO 配网的流程还不太合理，但是目前可以使用

	// if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCMQTTSTATE?"))
	// {
	// 	printf("cmd AT+TCMQTTSTATE exec err");
	// 	result = QCLOUD_ERR_FAILURE;
	// }
	// printf("TCMQTTSTATE ret lines %d\r\n", resp->line_counts);
	// printf("%s", resp->buf);
	// for (int i = 0; i < resp->line_counts - 1; i++)
	// {
	//     printf("%s\r\n", at_resp_get_line(resp, i + 1));
	// }
	// printf("TCMQTTSTATE end\r\n", resp->line_counts);

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCMQTTDISCONN"))
	{
		printf("cmd AT+TCMQTTDISCONN exec err");
		// result = QCLOUD_ERR_FAILURE;
	}
	
	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTARTSMART"))
	{
		printf("cmd AT+TCSTARTSMART exec err");
		// result = QCLOUD_ERR_FAILURE;
	}

	tos_sleep_ms(3000);

	DrawPic(0, 0, 128, 32, (uint8_t *)wifi_config_pic2);
	RefreshFullScreen();
	tos_sleep_ms(3000);

	// 等待60秒，结束配网
	CleanScreen();
	for(i=0; i<600; i++)
	{
		key = ReadEncoderKey();
		if(key == ENCODER_KEY_RELEASE)
		{
			break;
		}

		sprintf(&menu_line1[0], "配网中，请稍等 %02d", 60 - i / 10);
		PrintString12(0, 0, menu_line1);
		PrintString12(0, 14, "单击旋钮返回");
		RefreshFullScreen();
		tos_sleep_ms(70);
	}

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTOPSMART"))
	{
		// printf("cmd AT+TCSTARTSMART exec err");
		// result = QCLOUD_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
}

void power_switch(int32_t sw)
{
	power_sw = sw;
	if(sw)
	{
		LL_TIM_OC_SetCompareCH1(TIM3, brightness_yellow_out);
		LL_TIM_OC_SetCompareCH2(TIM3, brightness_white_out);
	}
	else
	{
		LL_TIM_OC_SetCompareCH1(TIM3, 0);
		LL_TIM_OC_SetCompareCH2(TIM3, 0);
	}
}

void set_brightness(int32_t brightness)
{
	brightness_value = brightness;

	brightness_yellow_out = brightness_yellow * brightness_value / 100;
	brightness_white_out = brightness_white * brightness_value / 100;
}

void set_color_temp(int32_t k)
{
	color_temp_value = k;
	// 色温转化为黄、白色灯管的亮度
	// 色温范围 2700 - 6500
	// 将色温2700-6500线性映射到0-100

	//线性插值法：(Y-Y0)/(Y1-Y0)=(X-X0)/(X1-X0)
	//即：Y=Y0+(Y1-Y0)/(X1-X0)*(X-X0)

	// y0 = 1000, y1 = 0, x0 = COLOR_TEMP_MIN, x1 = COLOR_TEMP_MAX
	brightness_yellow = 1000 + (0 - 1000.0) / (COLOR_TEMP_MAX - COLOR_TEMP_MIN) * (k - COLOR_TEMP_MIN);
	brightness_yellow_out = brightness_yellow * brightness_value / 100;

	// y0 = 0, y1 = 1000, x0 = COLOR_TEMP_MIN, x1 = COLOR_TEMP_MAX
	brightness_white = 0 + (1000 - 0.0) / (COLOR_TEMP_MAX - COLOR_TEMP_MIN) * (k - COLOR_TEMP_MIN);
	brightness_white_out = brightness_white * brightness_value / 100;
}


void user_task(void *arg);

#define USER_TASK_STACK_SIZE 1024
k_stack_t user_task_stack[USER_TASK_STACK_SIZE];
k_task_t user_task_t;

void start_user_task(void *arg)
{
	tos_task_create(&user_task_t,
					"user_task",
					(k_task_entry_t)user_task,
					arg,
					4,
					user_task_stack,
					USER_TASK_STACK_SIZE,
					0);
}

void user_task(void *arg)
{
	User_Init();

	power_switch(0);

	OLED_Init();
	OLED_Clear();
	OLED_Display_On();

	ws2812_reset();
	WS2812FX_init();
	WS2812FX_stop();

	// WS2812FX_setBrightness(10);
	// WS2812FX_setSpeed_param1(1000);
	// WS2812FX_setColor_u32(0x007B20);
	// WS2812FX_setMode(FX_MODE_STATIC);
	// WS2812FX_setMode_param1(20);
	// WS2812FX_start();

	while (1)
	{
		WS2812FX_service();
		menu_task();
		tos_task_delay(tos_millisec2tick(31));
	}
}
