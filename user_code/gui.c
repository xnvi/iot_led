#include "gui.h"
#include "oled.h"
#include <math.h>

#include <string.h>
// #include <stdio.h>

uint8_t GraphRAM[LCD_LENGTH * LCD_WIDTH / 8] = {0};

/*
显存的排列方式
0-191
192-383
384-575
576-767
768-959
960-1151
1152-1343
1344-1535

字节内纵向排列，低位在前，高位在后
D0
D1
D2
D3
D4
D5
D6
D7
*/


void CleanScreen()
{
	uint16_t i;
	for(i=0; i<LCD_LENGTH * LCD_WIDTH/8; i++)
	{
		GraphRAM[i] = 0x00;
	}
}

//这两个是高位在前
//void DrawPoint(uint8_t x, uint8_t y)
//{
//	//避免超出屏幕范围
//	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
//	{
//		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] | (0x80 >> (y & 0x07));
//	}
//}

//void CleanPoint(uint8_t x, uint8_t y)
//{
//	//避免超出屏幕范围
//	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
//	{
//		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] & (~(0x80 >> (y & 0x07)));
//	}
//}

//这两个是低位在前
void DrawPoint(int16_t x, int16_t y)
{
	//避免超出屏幕范围
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] | (0x01 << (y & 0x07));
	}
}

void CleanPoint(int16_t x, int16_t y)
{
	//避免超出屏幕范围
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] & (~(0x01 << (y & 0x07)));
	}
}

uint8_t ReadPoint(int16_t x, int16_t y)
{
	//避免超出屏幕范围
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		//读取像素
		//if(GraphRAM[x + LCD_LENGTH * (y >> 3)] & (0x80 >> (y & 0x07)))
		if(GraphRAM[x + LCD_LENGTH * (y >> 3)] & (0x01 << (y & 0x07)))
		{
			return 1;
		}
	}
	return 0;
}

void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{	
	//Bresenham算法
	int16_t dx = x2 - x1;
	int16_t dy = y2 - y1;
	int16_t ux = ((dx > 0) << 1) - 1;//x的增量方向，取或-1
	int16_t uy = ((dy > 0) << 1) - 1;//y的增量方向，取或-1
	int16_t x = x1, y = y1, eps;//eps为累加误差
	
	eps = 0;
	dx = abs(dx);
	dy = abs(dy);
//	(dx > 0) ? (dx) : (-dx);
//	(dy > 0) ? (dy) : (-dy);

	if(color)
	{
		if (dx > dy)
		{
			for (x = x1; x != x2+ux; x += ux)
			{
				DrawPoint(x, y);
				eps += dy;
				if ((eps << 1) >= dx)
				{
					y += uy; eps -= dx;
				}
			}
		}
		else
		{
			for (y = y1; y != y2+uy; y += uy)
			{
				DrawPoint(x, y);
				eps += dx;
				if ((eps << 1) >= dy)
				{
					x += ux; eps -= dy;
				}
			}
		}
	}
	else
	{
		if (dx > dy)
		{
			for (x = x1; x != x2+ux; x += ux)
			{
				CleanPoint(x, y);
				eps += dy;
				if ((eps << 1) >= dx)
				{
					y += uy; eps -= dx;
				}
			}
		}
		else
		{
			for (y = y1; y != y2+uy; y += uy)
			{
				CleanPoint(x, y);
				eps += dx;
				if ((eps << 1) >= dy)
				{
					x += ux; eps -= dy;
				}
			}
		}
	}
}


//模式0:黑色边框
//模式1:黑色填充
//模式2:白色边框
//模式3:白色填充
void DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t mode)
{
	uint16_t x;
	x = x1;
	switch(mode)
	{
	case 0:
		DrawLine(x1, y1, x2, y1, 1);
		DrawLine(x1, y2, x2, y2, 1);
		DrawLine(x1, y1, x1, y2, 1);
		DrawLine(x2, y1, x2, y2, 1);
		break;
	case 1:
		for( ; y1<y2+1; y1++)
		{
			for(x1=x; x1<x2+1; x1++)
			{
				DrawPoint(x1, y1);
			}
		}
		break;
	case 2:
		DrawLine(x1, y1, x2, y1, 0);
		DrawLine(x1, y2, x2, y2, 0);
		DrawLine(x1, y1, x1, y2, 0);
		DrawLine(x2, y1, x2, y2, 0);
		break;
	case 3:
		for( ; y1<y2+1; y1++)
		{
			for(x1=x; x1<x2+1; x1++)
			{
				CleanPoint(x1, y1);
			}
		}
		break;
	default: return;
	}
}

void RefreshFullScreen()
{
	// 画图范围0-127，0-7页
	OLED_DrawBMP(0, 0, 128, 8, GraphRAM);
}

//void ShowPartGraphRAM(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t zoom)
//{
//}

//纵向的数值都必须是8的整数倍
void DrawPicFast(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *pic)
{
	uint16_t i, j;
	uint8_t *p;
	
	for(i=y/8; i<height/8; i++)
	{
		p = (GraphRAM + LCD_LENGTH * i + x);
		for(j=x; j<x+width; j++)
		{
			*p = *pic;
			p++;
			pic++;
		}
	}
}

// 图片尺寸无要求，编码方式：阴码、逆向（低位在前）、列行式
void DrawPic(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *pic)
{
	uint16_t i, j, k;
	uint16_t count = 0, remainder;
	uint8_t pic_tmp;
	
	// 向上取整除法 c = (a + b - 1) / b;
	
	for(i=0; i<height; i+=8)
	{
		for(j=0; j<width; j++)
		{
			pic_tmp = pic[count];
			count++;
			if(height - i >= 8)
			{
				remainder = 8;
			}
			else
			{
				remainder = height % 8;
			}
			for(k=0; k<remainder; k++)
			{
				if(pic_tmp & 0x01)
				{
					DrawPoint(x+j, y+i+k);
				}
				else
				{
					CleanPoint(x+j, y+i+k);
				}
				pic_tmp >>= 1;
			}
		}
	}
}

void InverseColor(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	uint16_t a, b;

	//虽然实现了，但是效率不高
	for(a=x1; a<x2+1; a++)
	{
		for(b=y1; b<y2+1; b++)
		{
			//GraphRAM[a + LCD_LENGTH * (b >> 3)] = GraphRAM[a + LCD_LENGTH * (b >> 3)] ^ (0x80 >> (b & 0x07));//画点函数改成异或
			GraphRAM[a + LCD_LENGTH * (b >> 3)] = GraphRAM[a + LCD_LENGTH * (b >> 3)] ^ (0x01 << (b & 0x07));//画点函数改成异或
		}
	}
}
