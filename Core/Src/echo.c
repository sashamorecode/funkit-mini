/*
 * echo.c
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */

/*
 * delay.c
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */
#include "echo.h"
#include "main.h"
#include "filter_dsp.h"
#include <arm_math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static I2C_LCD_HandleTypeDef *lcdPtr;
static filterUnit filter;

//DMA buf ref
extern uint16_t rxBuf[BUF_SAMPLES];
extern uint16_t txBuf[BUF_SAMPLES];

extern float32_t srcLeft[SAMPLES / 2];
extern float32_t srcRight[SAMPLES / 2];
extern float32_t destLeft[SAMPLES / 2];
extern float32_t destRight[SAMPLES / 2];

// Delay Line

float32_t delayLeft[DELAY_BUF];
float32_t delayRight[DELAY_BUF];
extern volatile int delayPtr;

static char strBuff[20] = { "1" };
echoState eState = { 5000, 0., 0.5, 2000 };

void EchoInit(I2C_LCD_HandleTypeDef *lcdHandler, int audioFreq) {
	memset(delayLeft, 0, sizeof(delayLeft));
	memset(delayRight, 0, sizeof(delayRight));
	lcdPtr = lcdHandler;
	initFilter(&filter, audioFreq);
}

void doEcho(int b) {

	int startBuf = b * BUF_SAMPLES / 2;
	int endBuf = startBuf + BUF_SAMPLES / 2;

	int i = 0;
	for (int pos = startBuf; pos < endBuf; pos += 4) {
		srcLeft[i] = ((rxBuf[pos] << 16) | rxBuf[pos + 1]);
		srcRight[i] = ((rxBuf[pos + 2] << 16) | rxBuf[pos + 3]);
		i++;
	}

	i = 0;
	for (int pos = startBuf; pos < endBuf; pos += 4) {

		int32_t lval = srcLeft[i]
				+ delayLeft[delayPtr] * eState.mix * eState.feedback;
		int32_t rval = srcRight[i]
				+ delayRight[delayPtr] * eState.mix * eState.feedback;

		delayLeft[delayPtr] = srcLeft[i]
				+ delayLeft[delayPtr] * eState.feedback;
		delayRight[delayPtr] = srcRight[i]
				+ delayRight[delayPtr] * eState.feedback;

		float32_t lFilter, rFilter;
		if (eState.filterFreq < 19000) {
			doFilter(&filter, &delayLeft[delayPtr], &delayRight[delayPtr],
					&lFilter, &rFilter, 1);
			delayLeft[delayPtr] = lFilter;
			delayRight[delayPtr] = rFilter;
		}

		delayPtr = (delayPtr + 1) % eState.delayLen;
		txBuf[pos] = (lval >> 16) & 0xFFFF;
		txBuf[pos + 1] = lval & 0xFFFF;
		txBuf[pos + 2] = (rval >> 16) & 0xFFFF;
		txBuf[pos + 3] = rval & 0xFFFF;

		i++;
	}

}

void EchoUpdateState(masterState *mState) {
	int encoder1Switch = (HAL_GPIO_ReadPin(EncoderSW_GPIO_Port, EncoderSW_Pin)
			== GPIO_PIN_RESET);
	int encoder2Switch = (HAL_GPIO_ReadPin(EncoderSW2_GPIO_Port, EncoderSW2_Pin)
			== GPIO_PIN_RESET);
	int dirty = 0;
	if (!encoder1Switch && mState->encoder1Delta != 0) {
		eState.delayLen += mState->encoder1Delta * 200;
		if (eState.delayLen >= DELAY_BUF)
			eState.delayLen = DELAY_BUF - 1;
		else if (eState.delayLen < 1)
			eState.delayLen = 1;
		dirty = 1;
	} else if (mState->encoder1Delta != 0) {
		eState.mix += ((float) mState->encoder1Delta) * 0.01f;
		if (eState.mix < 0.)
			eState.mix = 0.;
		if (eState.mix > 1.)
			eState.mix = 1.;
		dirty = 1;
	}
	if (!encoder2Switch && mState->encoder2Delta != 0) {
		eState.feedback += ((float) mState->encoder2Delta) * 0.01f;
		if (eState.feedback < 0.)
			eState.feedback = 0.;
		else if (eState.feedback > 1.)
			eState.feedback = 1.;
		dirty = 1;
	} else if (mState->encoder2Delta != 0) {
		eState.filterFreq += ((float) mState->encoder2Delta) * 100.f;
		if (eState.filterFreq >= 20000)
			eState.filterFreq = 20000;
		if (eState.filterFreq < 1)
			eState.filterFreq = 1;
		compute_lowpass_coeffs(&filter, eState.filterFreq, 1.78);
		dirty = 1;
	}
	if (dirty) {
		EchoPrintState();
	}
}

void EchoPrintInit() {
	lcd_clear(lcdPtr);
	lcd_gotoxy(lcdPtr, 0, 0);
	lcd_puts(lcdPtr, "Echo Len smp:");

	lcd_gotoxy(lcdPtr, 0, 1);
	lcd_puts(lcdPtr, "Echo Feedback%:");

	lcd_gotoxy(lcdPtr, 0, 2);
	lcd_puts(lcdPtr, "Echo Wet%:");

	lcd_gotoxy(lcdPtr, 0, 3);
	lcd_puts(lcdPtr, "Filter Freq:");

	EchoPrintState();
}

void EchoPrintState() {

	sprintf(strBuff, "%d     ", eState.delayLen);
	strBuff[7] = '\0';
	lcd_gotoxy(lcdPtr, 13, 0);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d  ", (int) (eState.feedback * 100.));
	lcd_gotoxy(lcdPtr, 15, 1);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d  ", (int) (eState.mix * 100.));
	lcd_gotoxy(lcdPtr, 10, 2);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d  ", (int) (eState.filterFreq));
	strBuff[7] = '\0';
	lcd_gotoxy(lcdPtr, 12, 3);
	lcd_puts(lcdPtr, strBuff);

}
