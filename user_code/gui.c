#include "gui.h"
#include "oled.h"
#include <math.h>

#include <string.h>
// #include <stdio.h>

uint8_t GraphRAM[LCD_LENGTH * LCD_WIDTH / 8] = {0};

/*
�Դ�����з�ʽ
0-191
192-383
384-575
576-767
768-959
960-1151
1152-1343
1344-1535

�ֽ����������У���λ��ǰ����λ�ں�
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

//�������Ǹ�λ��ǰ
//void DrawPoint(uint8_t x, uint8_t y)
//{
//	//���ⳬ����Ļ��Χ
//	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
//	{
//		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] | (0x80 >> (y & 0x07));
//	}
//}

//void CleanPoint(uint8_t x, uint8_t y)
//{
//	//���ⳬ����Ļ��Χ
//	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
//	{
//		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] & (~(0x80 >> (y & 0x07)));
//	}
//}

//�������ǵ�λ��ǰ
void DrawPoint(int16_t x, int16_t y)
{
	//���ⳬ����Ļ��Χ
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] | (0x01 << (y & 0x07));
	}
}

void CleanPoint(int16_t x, int16_t y)
{
	//���ⳬ����Ļ��Χ
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		GraphRAM[x + LCD_LENGTH * (y >> 3)] = GraphRAM[x + LCD_LENGTH * (y >> 3)] & (~(0x01 << (y & 0x07)));
	}
}

uint8_t ReadPoint(int16_t x, int16_t y)
{
	//���ⳬ����Ļ��Χ
	if(x >= 0 && y >= 0 && x < LCD_LENGTH && y < LCD_WIDTH)
	{
		//��ȡ����
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
	//Bresenham�㷨
	int16_t dx = x2 - x1;
	int16_t dy = y2 - y1;
	int16_t ux = ((dx > 0) << 1) - 1;//x����������ȡ��-1
	int16_t uy = ((dy > 0) << 1) - 1;//y����������ȡ��-1
	int16_t x = x1, y = y1, eps;//epsΪ�ۼ����
	
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


//ģʽ0:��ɫ�߿�
//ģʽ1:��ɫ���
//ģʽ2:��ɫ�߿�
//ģʽ3:��ɫ���
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
	// ��ͼ��Χ0-127��0-7ҳ
	OLED_DrawBMP(0, 0, 128, 8, GraphRAM);
}

//void ShowPartGraphRAM(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t zoom)
//{
//}

//�������ֵ��������8��������
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

// ͼƬ�ߴ���Ҫ�󣬱��뷽ʽ�����롢���򣨵�λ��ǰ��������ʽ
void DrawPic(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *pic)
{
	uint16_t i, j, k;
	uint16_t count = 0, remainder;
	uint8_t pic_tmp;
	
	// ����ȡ������ c = (a + b - 1) / b;
	
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

	//��Ȼʵ���ˣ�����Ч�ʲ���
	for(a=x1; a<x2+1; a++)
	{
		for(b=y1; b<y2+1; b++)
		{
			//GraphRAM[a + LCD_LENGTH * (b >> 3)] = GraphRAM[a + LCD_LENGTH * (b >> 3)] ^ (0x80 >> (b & 0x07));//���㺯���ĳ����
			GraphRAM[a + LCD_LENGTH * (b >> 3)] = GraphRAM[a + LCD_LENGTH * (b >> 3)] ^ (0x01 << (b & 0x07));//���㺯���ĳ����
		}
	}
}
