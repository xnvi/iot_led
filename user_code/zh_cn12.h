/*
字库取模方式
目前用的 PCtoLCD2002
阴码 逆向（低位在前） 列行式

*/


#ifndef _ZH_CN12_H
#define _ZH_CN12_H

// #include "myinteger.h"
#include<stdint.h>

// 使用用户字库，或使用默认字库，默认字库为字频表前1024字，可覆盖大部分使用场景
#define USE_USER_FONT_TABLE 1
#define USE_DEFAULT_FONT_TABLE 0


typedef struct _FONT12_STRUCT
{
	uint8_t character[3];
	uint8_t reserve;
	uint8_t fontdata[24];
}font12_struct;


#if 0
// !!!! 二分查找要求所有汉字索引必须排序 !!!!
// const font12_struct font12_table[] = 
const font12_struct font12_table[] = 
{
	{"测", 0, {0x22,0x44,0xFE,0x02,0xFA,0x02,0xFE,0x00,0xFC,0x00,0xFF,0x00,0x04,0x02,0x09,0x04,0x03,0x04,0x09,0x00,0x01,0x08,0x0F,0x00},},
	{"试", 0, {0x11,0xF2,0x00,0x00,0x24,0xE4,0x24,0x04,0xFF,0x04,0x05,0x00,0x00,0x0F,0x04,0x00,0x04,0x07,0x02,0x00,0x03,0x04,0x0E,0x00},},
	{"文", 0, {0x04,0x04,0x1C,0x64,0x85,0x06,0x84,0x64,0x1C,0x04,0x04,0x00,0x08,0x08,0x04,0x04,0x02,0x01,0x02,0x04,0x04,0x08,0x08,0x00},},
	{"中", 0, {0x00,0xF8,0x88,0x88,0x88,0xFF,0x88,0x88,0x88,0xF8,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x01,0x00,0x00},},
};
#endif


#if USE_USER_FONT_TABLE

const font12_struct font12_table[] = 
{
	{"亮", 0, {0x82,0x82,0xBA,0xAA,0xAA,0xAB,0xAA,0xAA,0xBA,0x82,0x82,0x00,0x09,0x08,0x06,0x02,0x02,0x02,0x02,0x06,0x08,0x08,0x0D,0x00},},
	{"度", 0, {0x00,0xFE,0x0A,0x8A,0xBE,0xAA,0xAB,0xAA,0xBE,0x8A,0x0A,0x00,0x08,0x07,0x00,0x08,0x09,0x0A,0x04,0x04,0x0A,0x09,0x08,0x00},},
	{"色", 0, {0x08,0xF4,0x92,0x93,0x92,0xF2,0x92,0x9A,0x96,0xF0,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0E,0x00},},
	{"温", 0, {0x22,0x44,0x00,0xC0,0x5F,0xD5,0x55,0xD5,0x5F,0xC0,0x00,0x00,0x04,0x02,0x09,0x0F,0x08,0x0F,0x08,0x0F,0x08,0x0F,0x08,0x00},},
	{"返", 0, {0x10,0x11,0xF2,0x00,0xFE,0x0A,0x2A,0x4A,0x89,0x49,0x39,0x00,0x08,0x04,0x03,0x06,0x09,0x08,0x0A,0x09,0x08,0x09,0x0A,0x00},},
	{"回", 0, {0x00,0xFE,0x02,0x02,0xF2,0x92,0x92,0xF2,0x02,0x02,0xFE,0x00,0x00,0x0F,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x0F,0x00},},
	{"彩", 0, {0x4A,0x52,0x46,0xE9,0x51,0x4D,0x00,0x88,0x44,0x23,0x18,0x00,0x04,0x02,0x01,0x0F,0x01,0x02,0x08,0x08,0x04,0x02,0x01,0x00},},
	{"灯", 0, {0x38,0x00,0xFF,0x10,0x08,0x02,0x02,0x02,0xFE,0x02,0x02,0x00,0x08,0x06,0x01,0x02,0x0C,0x00,0x08,0x08,0x0F,0x00,0x00,0x00},},
	{"模", 0, {0x88,0x68,0xFF,0x48,0x02,0xFA,0xAF,0xAA,0xAF,0xFA,0x02,0x00,0x00,0x00,0x0F,0x00,0x0A,0x0A,0x06,0x03,0x06,0x0A,0x0A,0x00},},
	{"式", 0, {0x08,0x48,0x48,0xC8,0x48,0x48,0x08,0xFF,0x08,0x09,0x0A,0x00,0x08,0x08,0x08,0x07,0x04,0x04,0x04,0x00,0x03,0x04,0x0E,0x00},},
	{"设", 0, {0x10,0x11,0xF2,0x00,0x50,0xCF,0x41,0x41,0x4F,0xD0,0x10,0x00,0x00,0x00,0x07,0x02,0x08,0x08,0x05,0x02,0x05,0x08,0x08,0x00},},
	{"置", 1, {0x10,0x17,0xD5,0x55,0x57,0x7D,0x57,0x55,0xD5,0x17,0x10,0x00,0x08,0x08,0x0F,0x08,0x08,0x0E,0x08,0x08,0x0F,0x08,0x08,0x00},},
	{"关", 0, {0x40,0x48,0x49,0x4A,0x48,0xF8,0x48,0x4A,0x49,0x48,0x40,0x00,0x08,0x08,0x04,0x02,0x01,0x00,0x01,0x02,0x04,0x08,0x08,0x00},},
	{"闭", 0, {0x00,0xF9,0x02,0x20,0x20,0xA2,0xFA,0x22,0x22,0x02,0xFE,0x00,0x00,0x0F,0x00,0x02,0x01,0x04,0x07,0x00,0x08,0x08,0x0F,0x00},},
	{"配", 0, {0xFA,0x4A,0x3E,0x0A,0x3E,0x4A,0xFA,0x00,0xE2,0x22,0x3E,0x00,0x0F,0x05,0x05,0x05,0x05,0x05,0x0F,0x00,0x07,0x08,0x0E,0x00},},
	{"网", 0, {0xFF,0x09,0x91,0x61,0x91,0x09,0x91,0x61,0x91,0x09,0xFF,0x00,0x0F,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x08,0x09,0x0F,0x00},},
	{"中", 0, {0x00,0xF8,0x88,0x88,0x88,0xFF,0x88,0x88,0x88,0xF8,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x01,0x00,0x00},},
	{"请", 0, {0x11,0xF2,0x00,0x22,0xEA,0xAA,0xBF,0xAA,0xAA,0xEA,0x22,0x00,0x00,0x0F,0x04,0x00,0x0F,0x02,0x02,0x02,0x0A,0x0F,0x00,0x00},},
	{"稍", 0, {0x12,0xD2,0xFE,0x91,0x02,0xF4,0x50,0x5F,0x50,0x54,0xF2,0x00,0x01,0x00,0x0F,0x00,0x00,0x0F,0x01,0x01,0x01,0x09,0x0F,0x00},},
	{"等", 0, {0x44,0x53,0x52,0x56,0x52,0x7C,0x53,0xD2,0x56,0x52,0x42,0x00,0x01,0x01,0x03,0x05,0x01,0x09,0x09,0x0F,0x01,0x01,0x01,0x00},},
	{"单", 0, {0x00,0x7C,0x55,0x56,0x54,0xFC,0x54,0x56,0x55,0x7C,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x0F,0x01,0x01,0x01,0x01,0x01,0x00},},
	{"击", 0, {0x20,0xA4,0x24,0x24,0x24,0xFF,0x24,0x24,0x24,0xA4,0x20,0x00,0x00,0x07,0x04,0x04,0x04,0x07,0x04,0x04,0x04,0x0F,0x00,0x00},},
	{"旋", 0, {0x08,0xF8,0x49,0x4A,0xD0,0x08,0xD7,0x14,0xF4,0x14,0x34,0x00,0x08,0x07,0x08,0x08,0x07,0x08,0x07,0x08,0x0F,0x09,0x09,0x00},},
	{"钮", 0, {0x98,0xF7,0x94,0x94,0x42,0xC2,0x7E,0x42,0x42,0xFE,0x00,0x00,0x00,0x0F,0x04,0x08,0x08,0x0F,0x08,0x08,0x08,0x0F,0x08,0x00},},
	{"键", 0, {0x94,0xF3,0x92,0x64,0xDC,0x88,0xAA,0xFF,0xAA,0xBE,0x08,0x00,0x00,0x0F,0x04,0x0A,0x07,0x0A,0x0A,0x0F,0x0A,0x0A,0x0A,0x00},},
	{"连", 0, {0x21,0xE2,0x00,0x04,0x34,0x2C,0x27,0xF4,0x24,0x24,0x04,0x00,0x08,0x07,0x08,0x09,0x09,0x09,0x09,0x0F,0x09,0x09,0x09,0x00},},
	{"一", 0, {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},},
	{"二", 0, {0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00},},
	{"三", 0, {0x00,0x02,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x02,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00},},
	{"四", 0, {0x00,0xFE,0x02,0x82,0x7E,0x02,0x02,0x7E,0x82,0x82,0xFE,0x00,0x00,0x0F,0x05,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x0F,0x00},},
	{"五", 0, {0x02,0x22,0x22,0xE2,0x3E,0x22,0x22,0x22,0xE2,0x02,0x00,0x00,0x08,0x08,0x0E,0x09,0x08,0x08,0x08,0x08,0x0F,0x08,0x08,0x00},},
	{"六", 0, {0x10,0x10,0x10,0xD0,0x11,0x16,0x10,0x50,0x90,0x10,0x10,0x00,0x08,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x0C,0x00},},
	{"七", 0, {0x20,0x20,0x20,0x20,0xFF,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x08,0x0E,0x00},},
	{"八", 0, {0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x7F,0x80,0x00,0x00,0x00,0x08,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x08,0x00},},
	{"九", 0, {0x08,0x08,0x08,0xFF,0x08,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x08,0x04,0x03,0x00,0x00,0x00,0x00,0x07,0x08,0x08,0x0E,0x00},},
	{"十", 0, {0x20,0x20,0x20,0x20,0x20,0xFF,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00},},
	{"，", 0, {0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},},
	{"。", 0, {0x00,0x00,0xC0,0x20,0x20,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00},},
};

#endif // USE_USER_FONT_TABLE



#if USE_DEFAULT_FONT_TABLE

const font12_struct font12_table[] = 
{

};

#endif // USE_USER_FONT_TABLE

#endif // _ZH_CN12_H
