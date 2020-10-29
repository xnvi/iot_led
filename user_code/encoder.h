#ifndef _ENCODER_H
#define _ENCODER_H

// EncoderKeyRet ·µ»Ø±àÂëÆ÷°´¼ü×´Ì¬Öµ
#define ENCODER_KEY_NULL 0x00
#define ENCODER_KEY_RELEASE 0x01
#define ENCODER_KEY_PRESS 0x02

void EncoderInit(void);
void EncoderScan(void);
char ReadEncoder(void);
unsigned char ReadEncoderKey(void);

#endif
