/*
 * delay.c
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */
#include "delay.h"
#include "main.h"
#include <arm_math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



static I2C_LCD_HandleTypeDef* lcdPtr;



//DMA buf ref
extern uint16_t rxBuf[BUF_SAMPLES];
extern uint16_t txBuf[BUF_SAMPLES];

extern int32_t	srcLeft[SAMPLES/2];
extern int32_t	srcRight[SAMPLES/2];
extern int32_t	destLeft[SAMPLES/2];
extern int32_t	destRight[SAMPLES/2];

// Delay Line

extern float32_t	delayLeft[DELAY_BUF];
extern float32_t	delayRight[DELAY_BUF];
extern volatile int delayPtr;

static char strBuff[20] = {"1"};
delayState dState = {5000,0.,0,0};


void DelayInit(I2C_LCD_HandleTypeDef *lcdHandler){
	memset(delayLeft, 0, sizeof(delayLeft));
	memset(delayRight, 0, sizeof(delayRight));
	lcdPtr = lcdHandler;
	DelayPrintInit();
}

void doDelay(int b){
	int startBuf = b * BUF_SAMPLES / 2;
	int endBuf = startBuf + BUF_SAMPLES / 2;

	int i = 0;
	for ( int pos = startBuf ; pos < endBuf ; pos+=4 )
	{
		  srcLeft[i] = ( (rxBuf[pos]<<16)|rxBuf[pos+2] );
		  srcRight[i] =( (rxBuf[pos+1]<<16)|rxBuf[pos+3] );
		  i++;
	}

	i = 0;
	for ( int pos = startBuf ; pos < endBuf ; pos+=4 )
	  {
		  delayLeft[delayPtr] = srcLeft[i];
		  delayRight[delayPtr] = srcRight[i];
		  delayPtr = (delayPtr + 1) % dState.delayLen;
		  int lval = srcLeft[i] * (dState.mix - 1.) + delayLeft[delayPtr] * dState.mix;
		  int rval = srcRight[i] * (dState.mix - 1.) + delayRight[delayPtr] * dState.mix;
		  txBuf[pos] = (lval>>16)&0xFFFF;
		  txBuf[pos+1] = lval&0xFFFF;
		  txBuf[pos+2] = (rval>>16)&0xFFFF;
		  txBuf[pos+3] = rval&0xFFFF;

		  i++;
	  }
}



void DelayUpdateState(){
	int newEncoderVal = TIM1->CNT;
	int encoderDelata = newEncoderVal-dState.lastEncoderVal;
	if (abs(encoderDelata) > 20) {
		dState.lastEncoderVal = newEncoderVal;
		return;
	}
	GPIO_PinState encSW = HAL_GPIO_ReadPin(EncoderSW_GPIO_Port, EncoderSW_Pin);
	if(dState.currentParam == 0){
		if(encSW == GPIO_PIN_RESET){
			dState.currentParam = 1;
			dState.lastEncoderVal = newEncoderVal;
			return;
		}
		dState.delayLen += (encoderDelata)*10;
		//dState.delayLen = dState.delayLen % BUF_SAMPLES;
		if(dState.delayLen >= DELAY_BUF){
			dState.delayLen = DELAY_BUF -1;
		}
		if(dState.delayLen < 1){
			dState.delayLen = 1;
		}
	}
	else if(dState.currentParam == 1){
		if(encSW == GPIO_PIN_SET){
			dState.currentParam = 0;
			dState.lastEncoderVal = newEncoderVal;
			return;
		}
		dState.mix += ((float)(encoderDelata))*0.05;
		if(dState.mix < 0.) dState.mix = 0.;
		if(dState.mix > 1.) dState.mix = 1.;
	}

	if(dState.lastEncoderVal != newEncoderVal){
		DelayPrintState();
	}
	dState.lastEncoderVal = newEncoderVal;
}

void DelayPrintInit(){
	lcd_clear(lcdPtr);
	lcd_puts(lcdPtr, "Delay Length:");
	lcd_gotoxy(lcdPtr, 0, 2);
	lcd_puts(lcdPtr, "Delay Mix:");
	sprintf(strBuff, "%d smp    ", dState.delayLen);
	lcd_gotoxy(lcdPtr, 0, 1);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d    ", (int)(dState.mix*100.));
	lcd_gotoxy(lcdPtr, 0, 3);
	lcd_puts(lcdPtr, strBuff);
}

void DelayPrintState(){
	if(dState.currentParam == 0){
		sprintf(strBuff, "%d smp    ", dState.delayLen);
		lcd_gotoxy(lcdPtr, 0, 1);
		lcd_puts(lcdPtr, strBuff);
	}
	else {
		sprintf(strBuff, "%d     " , (int)(dState.mix*100.));
		lcd_gotoxy(lcdPtr, 0, 3);
		lcd_puts(lcdPtr, strBuff);
	}

}
