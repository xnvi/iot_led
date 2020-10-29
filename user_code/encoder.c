#include <stdio.h>
#include "stm32g0xx_ll_gpio.h"

#include "encoder.h"

//如果计数方向相反，用这个换一下
#define CHANGE_DIRECTION

// 编码器速率
// #define ENCODER_RATE_1_LOW  // 适用于静止时A、B相都为低电平
#define ENCODER_RATE_1_HIGH // 适用于静止时A、B相都为高电平
// #define ENCODER_RATE_2
// #define ENCODER_RATE_4

// 函数内部用于记录编码器按键的状态
#define ENCODER_KEY_STA_RELEASE 0x00
#define ENCODER_KEY_STA_IF_PUSH 0x01
#define ENCODER_KEY_STA_PUSH 0x02
#define ENCODER_KEY_STA_IF_RELEASE 0x03


static unsigned char EncoderKeyRet = ENCODER_KEY_NULL;
static unsigned char EncoderKeyState = ENCODER_KEY_STA_RELEASE; // 见宏定义

static char EncoderCount = 0;
static unsigned char EncoderState = 0;

#define ENCODER_IO_KEY LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1)
#define ENCODER_IO_A LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_2)
#define ENCODER_IO_B LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_3)


void EncoderInit()
{
	//IO初始化
	// ENCODER_IO_A = 1;
	// ENCODER_IO_B = 1;
	// ENCODER_IO_KEY = 1;
}

// 编码器扫描函数，放到定时器中断服务函数里
void EncoderScan(void)
{
	// EncoderPA = ENCODER_IO_A;
	// EncoderPB = ENCODER_IO_B;
	
	//按键扫描
	if((EncoderKeyState == ENCODER_KEY_STA_RELEASE) && ENCODER_IO_KEY == 0)
	{
		EncoderKeyState = ENCODER_KEY_STA_IF_PUSH;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_IF_PUSH) && ENCODER_IO_KEY == 0)
	{
		EncoderKeyState = ENCODER_KEY_STA_PUSH;
		// 按键第一次按下
		EncoderKeyRet = ENCODER_KEY_PRESS;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_PUSH) && ENCODER_IO_KEY == 0)
	{
		// 按键持续按下
		EncoderKeyRet = ENCODER_KEY_PRESS;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_PUSH) && ENCODER_IO_KEY == 1)
	{
		EncoderKeyState = ENCODER_KEY_STA_IF_RELEASE;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_IF_RELEASE) && ENCODER_IO_KEY == 1)
	{
		EncoderKeyState = ENCODER_KEY_STA_RELEASE;
		// 按键释放
		EncoderKeyRet = ENCODER_KEY_RELEASE;
	}
	
	//	Read new state bits
	if (ENCODER_IO_A)
	{
		EncoderState |= 0x02;
	}
	if (ENCODER_IO_B)
	{
		EncoderState |= 0x01;
	}

#ifdef ENCODER_RATE_1_LOW
	switch (EncoderState)
	{
		case 0x01:
			EncoderCount -= 1;
			break;
		case 0x02:
			EncoderCount += 1;
			break;
		default:
			break;
	}
#endif

#ifdef ENCODER_RATE_1_HIGH
	switch (EncoderState)
	{
		case 0x0E:
			EncoderCount -= 1;
			break;
		case 0x0D:
			EncoderCount += 1;
			break;
		default:
			break;
	}
#endif

#ifdef ENCODER_RATE_2
	switch (EncoderState)
	{
		case 0x01:
		case 0x0E:
			EncoderCount -= 1;
			break;
		case 0x02:
		case 0x0D:
			EncoderCount += 1;
			break;
		default:
			break;
	}
#endif

#ifdef ENCODER_RATE_4
	switch (EncoderState)
	{
		case 0x01:
		case 0x07:
		case 0x08:
		case 0x0E:
			EncoderCount -= 1;
			break;
		case 0x02:
		case 0x04:
		case 0x0B:
		case 0x0D:
			EncoderCount += 1;
			break;
		case 0x03:
		case 0x0C:
			EncoderCount -= 2;
			break;
		case 0x06:
		case 0x09:
			EncoderCount += 2;
			break;
		default:
		//case 0x00:
		//case 0x05:
		//case 0x0A:
		//case 0x0F:
			//EncoderCount += 0;
			break;
	}
#endif
	
	// 更新状态
	EncoderState <<= 2;
	// 清除其他状态位
	EncoderState &= 0x0C;

	return;
}

char ReadEncoder()
{
	char c = 0;
	
	if(EncoderCount == 0)
	{
		return 0;
	}
	else 
	{
		#ifdef CHANGE_DIRECTION
		c = -EncoderCount;
		#else
		c = EncoderCount;
		#endif
		EncoderCount = 0;//读取按键后将编码器计数清零
		return c;
	}
}

unsigned char ReadEncoderKey()
{
	char c = 0;
	
	if(EncoderKeyRet == ENCODER_KEY_NULL)
	{
		return 0;
	}
	else 
	{
		c = EncoderKeyRet;
		EncoderKeyRet = ENCODER_KEY_NULL; // 读取按键后将按键清零
		return c;
	}
}

