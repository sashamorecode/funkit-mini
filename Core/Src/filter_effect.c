/*
 * filter.c
 *
 *  Created on: May 18, 2025
 *      Author: sasha
 */

#include "filter_effect.h"
#include <math.h>
#include <arm_math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static I2C_LCD_HandleTypeDef *lcdPtr;
static filterUnit filter;
static filterEffectState fState = { 10000.f, 1.f, 1.f, LP, 0};
static char strBuff[20] = { "1" };
//DMA buf ref
extern uint16_t rxBuf[BUF_SAMPLES];
extern uint16_t txBuf[BUF_SAMPLES];

extern float32_t srcLeft[SAMPLES / 2];
extern float32_t srcRight[SAMPLES / 2];
extern float32_t destLeft[SAMPLES / 2];
extern float32_t destRight[SAMPLES / 2];

void FilterEffectInit(I2C_LCD_HandleTypeDef *lcd, int audioFreq) {
	lcdPtr = lcd;
	initFilter(&filter, audioFreq);

}

void doFilterEffect(int b) {
	int startBuf = b * BUF_SAMPLES / 2;
	int endBuf = startBuf + BUF_SAMPLES / 2;

	int i = 0;
	for (int pos = startBuf; pos < endBuf; pos += 4) {
		srcLeft[i] = ((rxBuf[pos] << 16) | rxBuf[pos + 1]);
		srcRight[i] = ((rxBuf[pos + 2] << 16) | rxBuf[pos + 3]);
		i++;
	}
	doFilter(&filter, srcLeft, srcRight, destLeft, destRight, i);
	i = 0;
	for (int pos = startBuf; pos < endBuf; pos += 4) {
		int32_t lval = destLeft[i] * fState.mix + srcLeft[i] * (1 - fState.mix);
		int32_t rval = destRight[i] * fState.mix
				+ srcRight[i] * (1 - fState.mix);
		txBuf[pos] = (lval >> 16) & 0xFFFF;
		txBuf[pos + 1] = lval & 0xFFFF;
		txBuf[pos + 2] = (rval >> 16) & 0xFFFF;
		txBuf[pos + 3] = rval & 0xFFFF;

		i++;
	}
}
void FilterEffectUpdateState(masterState *mState) {
	GPIO_PinState encoder1Switch = (HAL_GPIO_ReadPin(EncoderSW_GPIO_Port,
			EncoderSW_Pin) == GPIO_PIN_RESET);
	GPIO_PinState encoder2Switch = (HAL_GPIO_ReadPin(EncoderSW2_GPIO_Port,
				EncoderSW2_Pin) == GPIO_PIN_RESET);

	int dirty = 0;
	if (mState->encoder1Delta != 0) {
		if (!encoder1Switch) {
			fState.freq += ((float) mState->encoder1Delta) * (fState.freq/100.f);
			if (fState.freq >= 20000)
				fState.freq = 20000;
			if (fState.freq < 20)
				fState.freq = 20;
			else
				dirty += 2;
		} else {
			if (mState->encoder1Delta != 0) {
				fState.mix += ((float) (mState->encoder1Delta)) * 0.01;
				if (fState.mix < 0.)
					fState.mix = 0.;
				if (fState.mix > 1.)
					fState.mix = 1.;
				else
					dirty += 1;
			}
		}
	}
	if (mState->encoder2Delta != 0) {
		if (!encoder2Switch) {
			fState.res += ((float) mState->encoder2Delta) * 0.05f;
			if (fState.res < 0.1f)
				fState.res = 0.1f;
			if (fState.res > 16.f)
				fState.res = 16.f;
			else
				dirty += 2;
		} else {
			if (mState->encoder2Delta > 0) {
				if (fState.filterTypeTracker > 2
						&& fState.type < NUM_FILTER_TYPES - 1){
					fState.type++;
					fState.filterTypeTracker=0;
				}
				else fState.filterTypeTracker++;
			} else {
				if (fState.filterTypeTracker < -2 && fState.type > 0){
					fState.type--;
					fState.filterTypeTracker = 0;
				}
				else fState.filterTypeTracker--;
			}
			dirty += 2;
		}
	}
	if (dirty != 0) {
		FilterEffectPrintState();
	}
	if (dirty >= 2) {
		compute_lowpass_coeffs(&filter, fState.freq, fState.res);
	}
}

void FilterEffectPrintInit() {
	lcd_clear(lcdPtr);
	lcd_gotoxy(lcdPtr, 0, 0);
	lcd_puts(lcdPtr, "Filter Freq:");

	lcd_gotoxy(lcdPtr, 0, 1);
	lcd_puts(lcdPtr, "Filter Res :");

	lcd_gotoxy(lcdPtr, 0, 2);
	lcd_puts(lcdPtr, "Filter Type:");

	lcd_gotoxy(lcdPtr, 0, 3);
	lcd_puts(lcdPtr, "Filter Mix%:");

	FilterEffectPrintState();
}

void FilterEffectPrintState() {

	sprintf(strBuff, "%d     ", (int) fState.freq);

	lcd_gotoxy(lcdPtr, 12, 0);
	lcd_puts(lcdPtr, strBuff);

	sprintf(strBuff, "%f  ", (fState.res));
	strBuff[5] = '\0';
	lcd_gotoxy(lcdPtr, 12, 1);
	lcd_puts(lcdPtr, strBuff);

	switch (fState.type) {
	case LP:
		sprintf(strBuff, "LP");
		break;
	case HP:
		sprintf(strBuff, "HP");
		break;
	case BP:
		sprintf(strBuff, "BP");
		break;
	}
	lcd_gotoxy(lcdPtr, 12, 2);
	lcd_puts(lcdPtr, strBuff);

	sprintf(strBuff, "%d  ", (int) (fState.mix * 100.));
	lcd_gotoxy(lcdPtr, 12, 3);
	lcd_puts(lcdPtr, strBuff);

}
