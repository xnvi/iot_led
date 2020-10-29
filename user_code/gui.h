#ifndef __GUI_H
#define __GUI_H

#include<stdint.h>

//屏幕的长宽参数
#define LCD_LENGTH 128
#define LCD_WIDTH 32

/*****************************图形相关**************************/
extern uint8_t GraphRAM[];

void CleanScreen(void);
void DrawPoint(int16_t x, int16_t y);
void CleanPoint(int16_t x, int16_t y);
uint8_t ReadPoint(int16_t x, int16_t y);
void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);
void DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t mode);
void InverseColor(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void RefreshFullScreen(void);
void DrawPic(int16_t x, int16_t y, int16_t length, int16_t width, uint8_t *pic);
void DrawPicFast(int16_t x, int16_t y, int16_t length, int16_t width, uint8_t *pic);

#endif
