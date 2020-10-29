#ifndef __FONT_LITE_H
#define __FONT_LITE_H

#include<stdint.h>

uint8_t encodingCheck(uint8_t *pchar);

void DrawFontInGram12(uint8_t type, uint8_t *font, int16_t x, int16_t y);
void DrawFontInGram16(uint8_t type, uint8_t *font, int16_t x, int16_t y);

uint8_t* font12_search(uint8_t *pchar);
uint8_t* font16_search(uint8_t *pchar);

void PrintString12(int16_t x, int16_t y, char *string);
void PrintString16(int16_t x, int16_t y, char *string);

#endif
