#include <stdio.h>
#include "stm32g0xx_ll_gpio.h"

#include "encoder.h"

//������������෴���������һ��
#define CHANGE_DIRECTION

// ����������
// #define ENCODER_RATE_1_LOW  // �����ھ�ֹʱA��B�඼Ϊ�͵�ƽ
#define ENCODER_RATE_1_HIGH // �����ھ�ֹʱA��B�඼Ϊ�ߵ�ƽ
// #define ENCODER_RATE_2
// #define ENCODER_RATE_4

// �����ڲ����ڼ�¼������������״̬
#define ENCODER_KEY_STA_RELEASE 0x00
#define ENCODER_KEY_STA_IF_PUSH 0x01
#define ENCODER_KEY_STA_PUSH 0x02
#define ENCODER_KEY_STA_IF_RELEASE 0x03


static unsigned char EncoderKeyRet = ENCODER_KEY_NULL;
static unsigned char EncoderKeyState = ENCODER_KEY_STA_RELEASE; // ���궨��

static char EncoderCount = 0;
static unsigned char EncoderState = 0;

#define ENCODER_IO_KEY LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1)
#define ENCODER_IO_A LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_2)
#define ENCODER_IO_B LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_3)


void EncoderInit()
{
	//IO��ʼ��
	// ENCODER_IO_A = 1;
	// ENCODER_IO_B = 1;
	// ENCODER_IO_KEY = 1;
}

// ������ɨ�躯�����ŵ���ʱ���жϷ�������
void EncoderScan(void)
{
	// EncoderPA = ENCODER_IO_A;
	// EncoderPB = ENCODER_IO_B;
	
	//����ɨ��
	if((EncoderKeyState == ENCODER_KEY_STA_RELEASE) && ENCODER_IO_KEY == 0)
	{
		EncoderKeyState = ENCODER_KEY_STA_IF_PUSH;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_IF_PUSH) && ENCODER_IO_KEY == 0)
	{
		EncoderKeyState = ENCODER_KEY_STA_PUSH;
		// ������һ�ΰ���
		EncoderKeyRet = ENCODER_KEY_PRESS;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_PUSH) && ENCODER_IO_KEY == 0)
	{
		// ������������
		EncoderKeyRet = ENCODER_KEY_PRESS;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_PUSH) && ENCODER_IO_KEY == 1)
	{
		EncoderKeyState = ENCODER_KEY_STA_IF_RELEASE;
	}
	else if((EncoderKeyState == ENCODER_KEY_STA_IF_RELEASE) && ENCODER_IO_KEY == 1)
	{
		EncoderKeyState = ENCODER_KEY_STA_RELEASE;
		// �����ͷ�
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
	
	// ����״̬
	EncoderState <<= 2;
	// �������״̬λ
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
		EncoderCount = 0;//��ȡ�����󽫱�������������
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
		EncoderKeyRet = ENCODER_KEY_NULL; // ��ȡ�����󽫰�������
		return c;
	}
}

