#include "gui.h"
// #include "ascii8.h"
#include "ascii12.h"
#include "ascii16.h"
#include "zh_cn12.h"
#include "zh_cn16.h"
#include <stdio.h>


#ifndef LCD_WIDTH
#error "LCD_WIDTH NOT DEFINED"
#endif

#ifndef LCD_LENGTH
#error "LCD_LENGTH NOT DEFINED"
#endif


const uint8_t ErrPic12[24] = {
	0x00,0x66,0x02,0xA8,0x50,0xAA,0x52,0xA8,
	0x50,0x02,0x66,0x00,0x00,0x06,0x04,0x00,
	0x01,0x04,0x05,0x00,0x01,0x04,0x06,0x00,
};

const uint8_t ErrPic16[32] = {
	0x00,0x66,0x02,0xA8,0x50,0xAA,0x52,0xA8,
	0x50,0xAA,0x52,0xA8,0x50,0x02,0x66,0x00,
	0x00,0x66,0x40,0x0A,0x15,0x4A,0x55,0x0A,
	0x15,0x4A,0x55,0x0A,0x15,0x40,0x66,0x00,
};

const uint16_t font12_num = sizeof(font12_table) / sizeof(font12_struct);
const uint16_t font16_num = sizeof(font16_table) / sizeof(font16_struct);

void DrawFontInGram12(uint8_t type, uint8_t *font, int16_t x, int16_t y);
void DrawFontInGram16(uint8_t type, uint8_t *font, int16_t x, int16_t y);


uint8_t* font12_search(uint8_t *pchar)
{
// 1二分查找 0线性查找
// !!!! 二分查找要求所有汉字索引必须排序 !!!!
#if 0

	// 二分查找版
	uint16_t low, high, mid, aim, now;
	// 直接将传入的字符作为一个uint16_t的数字处理
	aim = (pchar[0]) + (pchar[1] << 8); // 避免地址不对齐时产生 HardFault
	low = 0;
	high = font12_num;

// sprintf(ComTxBuf, "font_search input #%x \r\n", aim);
// MyPrint(ComTxBuf);

	while (low <= high)
	{
		mid = low + (high - low) / 2;
		// now = (font12_table[mid].character[0] << 8) + font12_table[mid].character[1]; // 下面的方式更好
		now = *(uint16_t *)&font12_table[mid].character; // 将字符作为一个uint16_t的数字处理

		// sprintf(ComTxBuf, "font_search now #%x \r\n", now);
		// MyPrint(ComTxBuf);

		if (aim == now)
		{
			return (uint8_t*)&font12_table[mid].fontdata;
		}
		else if (aim < now)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}	
	}
	return (uint8_t *)ErrPic12;
	// return NULL;

#else

	// 线性查找版
	uint16_t aim, now;
	uint16_t i = 0;

	// 直接将传入的字符作为一个uint16_t的数字处理
	aim = (pchar[0]) + (pchar[1] << 8); // 避免地址不对齐时产生 HardFault
	// aim = *(uint16_t *)pchar; // 8位单片机使用没有问题，32位单片机如果字符串中汉字起始字节不是双数地址，会导致 HardFault

	// 自减方式效率高，但是第一个字会丢，除非将第一个字做成空白字符
	while (i < font12_num)
	{
		// now = (font12_table[i].character[0] << 8) + font12_table[mid].character[1]; // 下面的方式更好
		now = *(uint16_t *)&font12_table[i].character; // 将字符作为一个uint16_t的数字处理
		if(now == aim)
		{
			return (uint8_t*)&font12_table[i].fontdata;
		}
		i++;
	}

// sprintf(ComTxBuf, "font not found #%x \r\n", aim);
// MyPrint(ComTxBuf);

	return (uint8_t *)ErrPic12;
	// return NULL;

#endif
}

// uint8_t* font_search(uint8_t ch, uint8_t cl)
uint8_t* font16_search(uint8_t *pchar)
{
// 1二分查找 0线性查找
// !!!! 二分查找要求所有汉字索引必须排序 !!!!
#if 0

	// 二分查找版
	uint16_t low, high, mid, aim, now;
	// aim = (ch << 8) + cl; // 下面的方式更好
	// 直接将传入的字符作为一个uint16_t的数字处理
	aim = (pchar[0]) + (pchar[1] << 8); // 避免地址不对齐时产生 HardFault
	low = 0;
	high = font16_num;

// sprintf(ComTxBuf, "font_search input #%x \r\n", aim);
// MyPrint(ComTxBuf);

	while (low <= high)
	{
		mid = low + (high - low) / 2;
		// now = (font16_table[mid].character[0] << 8) + font16_table[mid].character[1]; // 下面的方式更好
		now = *(uint16_t *)&font16_table[mid].character; // 将字符作为一个uint16_t的数字处理

		// sprintf(ComTxBuf, "font_search now #%x \r\n", now);
		// MyPrint(ComTxBuf);

		if (aim == now)
		{
			return (uint8_t*)&font16_table[mid].fontdata;
		}
		else if (aim < now)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}	
	}
	return (uint8_t *)ErrPic16;
	// return NULL;

#else

	// 线性查找版
	uint16_t aim, now;
	uint16_t i = 0;
	// 直接将传入的字符作为一个uint16_t的数字处理
	aim = (pchar[0]) + (pchar[1] << 8); // 避免地址不对齐时产生 HardFault

	// 自减方式效率高，但是第一个字会丢，除非将第一个字做成空白字符
	while (i < font16_num)
	{
		// now = (font16_table[i].character[0] << 8) + font16_table[mid].character[1]; // 下面的方式更好
		now = *(uint16_t *)&font16_table[i].character; // 将字符作为一个uint16_t的数字处理
		if(now == aim)
		{
			return (uint8_t*)&font16_table[i].fontdata;
		}
		i++;
	}

// sprintf(ComTxBuf, "font not found #%x \r\n", aim);
// MyPrint(ComTxBuf);

	return (uint8_t *)ErrPic16;
	// return NULL;

#endif
}

// 12号字体
void DrawFontInGram12(uint8_t type, uint8_t *font, int16_t x, int16_t y)
{
	uint8_t i, j;
	uint8_t tmpfont; // 暂存一字节的字符图形数据
	
	if(type == 'c')
	{
		for(j=0; j<12; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j, y+i);
				}
				else
				{
					CleanPoint(x+j, y+i);
				}
				tmpfont >>= 1;
			}
		}
		for(; j<24; j++)
		{
			tmpfont = font[j];
			for(i=0; i<4; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j-12, y+i+8);
				}
				else
				{
					CleanPoint(x+j-12, y+i+8);
				}
				tmpfont >>= 1;
			}
		}
	}
	
	if(type == 'e')
	{
		for(j=0; j<6; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j, y+i);
				}
				else
				{
					CleanPoint(x+j, y+i);
				}
				tmpfont >>= 1;
			}
		}
		for(; j<12; j++)
		{
			tmpfont = font[j];
			for(i=0; i<4; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j-6, y+i+8);
				}
				else
				{
					CleanPoint(x+j-6, y+i+8);
				}
				tmpfont >>= 1;
			}
		}
	}
}


//高位在前
//unsigned char type：字符类型，'c'中，'e'英
//unsigned char *font：16*16像素的字符图形数组
//unsigned char x, unsigned char y：字符坐标x、y
void DrawFontInGram16(uint8_t type, uint8_t *font, int16_t x, int16_t y)
{
	uint8_t i, j;
	uint8_t tmpfont; // 暂存一字节的字符图形数据
	
	if(type == 'c')
	{
		for(j=0; j<16; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j, y+i);
				}
				else
				{
					CleanPoint(x+j, y+i);
				}
				tmpfont >>= 1;
			}
		}
		for(; j<32; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j-16, y+i+8);
				}
				else
				{
					CleanPoint(x+j-16, y+i+8);
				}
				tmpfont >>= 1;
			}
		}
	}
	
	if(type == 'e')
	{
		for(j=0; j<8; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j, y+i);
				}
				else
				{
					CleanPoint(x+j, y+i);
				}
				tmpfont >>= 1;
			}
		}
		for(; j<16; j++)
		{
			tmpfont = font[j];
			for(i=0; i<8; i++)
			{
				if(tmpfont & 0x01)
				{
					DrawPoint(x+j-8, y+i+8);
				}
				else
				{
					CleanPoint(x+j-8, y+i+8);
				}
				tmpfont >>= 1;
			}
		}
	}
}

// 12号字体
void PrintString12(int16_t x, int16_t y, char *string)
{
	// uint8_t i;
	uint8_t *pchar;
	uint8_t *pfont; // 保存找到的字体数据地址
	
	pchar = (uint8_t *)string;//注意这个类型转换

	while(*pchar != 0x00)
	{
		//if(*pchar > 31 && *pchar < 128)
		if(*pchar <= 31 || *pchar == 127)
		{
			 // 控制字符直接跳过
			pchar++;
			continue;
		}
		if(*pchar > 31 && *pchar < 127)//这里应该是小于127，第127个字符是del
		{
			// 直接算出字符数据的地址
			pfont = (uint8_t *)&ASCII12[(*pchar - 32) * 12];

			//以下代码实现自动换行
			if(LCD_LENGTH - x < 6)
			{
				y += 13; // 换行时多加一像素，以免和上一行汉字混在一起看不清
				x = 0;
			}

			//如果最后一行显示不下，直接返回
			if(y >= LCD_WIDTH)
			{
				return;
			}
			
			//显示字符
			DrawFontInGram12('e', pfont, x, y);
			x += 6;
			
			pchar++;
		}
		else
		{
			pfont = font12_search(pchar);

			//以下代码实现自动换行
			if(LCD_LENGTH - x < 12)
			{
				y += 13; // 换行时多加一像素，以免和上一行汉字混在一起看不清
				x = 0;
			}

			//如果最后一行显示不下，直接返回
			if(y >= LCD_WIDTH)
			{
				return;
			}

			//显示字符
			DrawFontInGram12('c', pfont, x, y);
			x += 12;

			//汉字两字节，指针加2
			pchar += 2;
		}
	}
}

//unsigned char x, unsigned char y：字符坐标x、y
//unsigned char *pchar：字符数组
//超出屏幕显示范围的字符将不显示
void PrintString16(int16_t x, int16_t y, char *string)
{
	// uint8_t i;
	uint8_t *pchar;
	uint8_t *pfont; // 保存找到的字体数据地址
	
	pchar = (uint8_t *)string;//注意这个类型转换

	while(*pchar != 0x00)
	{
		//if(*pchar > 31 && *pchar < 128)
		if(*pchar > 31 && *pchar < 127)//这里应该是小于127，第127个字符是del
		{
			// 直接算出字符数据的地址
			pfont = (uint8_t *)&ASCII16[(*pchar - 32) * 16];

			//以下代码实现自动换行
			if(LCD_LENGTH - x < 8)
			{
				// y += 17; // 换行时多加一像素，以免和上一行汉字混在一起看不清
				y += 16;
				x = 0;
			}

			//如果最后一行显示不下，直接返回
			if(y >= LCD_WIDTH)
			{
				return;
			}
			
			//显示字符
			DrawFontInGram16('e', pfont, x, y);
			x += 8;
			
			pchar++;
		}
		else
		{
			pfont = font16_search(pchar);

			//以下代码实现自动换行
			if(LCD_LENGTH - x < 16)
			{
				// y += 17; // 换行时多加一像素，以免和上一行汉字混在一起看不清
				y += 16;
				x = 0;
			}

			//如果最后一行显示不下，直接返回
			if(y >= LCD_WIDTH)
			{
				return;
			}

			//显示字符
			DrawFontInGram16('c', pfont, x, y);
			x += 16;

			//汉字两字节，指针加2
			pchar += 2;
		}
	}
}

