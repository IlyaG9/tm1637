#include "tm1637.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

const char segmentMap[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0-7
		0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, // 8-9, A-F
		0x00 };

void tm1637ClkHigh(void) {
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}
void tm1637ClkLow(void) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}
void tm1637DioHigh(void) {
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}

void tm1637DioLow(void) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
}
void tm1637DelayUsec(unsigned int i) {
	for (; i > 0; i--) {
		for (int j = 0; j < 10; ++j) {
			__asm__ __volatile__("nop\n\t":::"memory");
		}
	}
}

void tm1637WriteByte(unsigned char b) {
	for (int i = 0; i < 8; ++i) {
		tm1637ClkLow();
		if (b & 0x01) {
			tm1637DioHigh();
		} else {
			tm1637DioLow();
		}
		tm1637DelayUsec(3);
		b >>= 1;
		tm1637ClkHigh();
		tm1637DelayUsec(3);
	}
}
void tm1637Start(void) {
	tm1637ClkHigh();
	tm1637DioHigh();
	tm1637DelayUsec(2);
	tm1637DioLow();
}
void tm1637Init(void) {

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef clkPin;
	clkPin.GPIO_Pin = GPIO_Pin_4;
	clkPin.GPIO_Mode = GPIO_Mode_Out_OD;
	clkPin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &clkPin);

	GPIO_InitTypeDef dioPin;
	dioPin.GPIO_Pin = GPIO_Pin_3;
	dioPin.GPIO_Mode = GPIO_Mode_Out_OD;
	dioPin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &dioPin);

	tm1637SetBrightness(8);
}

void tm1637Stop(void) {
	tm1637ClkLow();
	tm1637DelayUsec(2);
	tm1637DioLow();
	tm1637DelayUsec(2);
	tm1637ClkHigh();
	tm1637DelayUsec(2);
	tm1637DioHigh();
}

void tm1637ReadResult(void) {
	tm1637ClkLow();
	tm1637DelayUsec(5);
	// while (dio); // We're cheating here and not actually reading back the response.
	tm1637ClkHigh();
	tm1637DelayUsec(2);
	tm1637ClkLow();
}

void tm1637SetBrightness(char brightness) {
	// Brightness command:
	// 1000 0XXX = display off
	// 1000 1BBB = display on, brightness 0-7
	// X = don't care
	// B = brightness
	tm1637Start();
	tm1637WriteByte(0x87 + brightness);
	tm1637ReadResult();
	tm1637Stop();
}

void tm1627Display(int v, int displaySeparator) {
	unsigned char digitArr[4];
	for (int i = 0; i < 4; ++i) {
		digitArr[i] = segmentMap[v % 10];
		if (i == 2 && displaySeparator) {
			digitArr[i] |= 1 << 7;
		}
		v /= 10;
	}

	tm1637Start();
	tm1637WriteByte(0x40);
	tm1637ReadResult();
	tm1637Stop();

	tm1637Start();
	tm1637WriteByte(0xc0);
	tm1637ReadResult();

	for (int i = 0; i < 4; ++i) {
		tm1637WriteByte(digitArr[3 - i]);
		tm1637ReadResult();
	}

	tm1637Stop();
}





