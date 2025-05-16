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
#include <arm_math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



static I2C_LCD_HandleTypeDef* lcdPtr;



//DMA buf ref
extern uint16_t rxBuf[BUF_SAMPLES];
extern uint16_t txBuf[BUF_SAMPLES];

extern float32_t	srcLeft[SAMPLES/2];
extern float32_t	srcRight[SAMPLES/2];
extern float32_t	destLeft[SAMPLES/2];
extern float32_t	destRight[SAMPLES/2];

// Delay Line

float32_t	delayLeft[DELAY_BUF];
float32_t	delayRight[DELAY_BUF];
extern volatile int delayPtr;

static char strBuff[20] = {"1"};
echoState eState = {5000,0.,0.5,0,0,0};


void EchoInit(I2C_LCD_HandleTypeDef *lcdHandler){
	memset(delayLeft, 0, sizeof(delayLeft));
	memset(delayRight, 0, sizeof(delayRight));
	lcdPtr = lcdHandler;
}

void doEcho(int b){

	int startBuf = b * BUF_SAMPLES / 2;
	int endBuf = startBuf + BUF_SAMPLES / 2;

	int i = 0;
	for ( int pos = startBuf ; pos < endBuf ; pos+=4 )
	{
		  srcLeft[i] = ( (rxBuf[pos]<<16)|rxBuf[pos+1] );
		  srcRight[i] =( (rxBuf[pos+2]<<16)|rxBuf[pos+3] );
		  i++;
	}

	i = 0;
	for ( int pos = startBuf ; pos < endBuf ; pos+=4 )
	  {

		  int32_t lval = srcLeft[i] + delayLeft[delayPtr] * eState.mix * eState.feedback;
		  int32_t rval = srcRight[i] + delayRight[delayPtr] * eState.mix * eState.feedback;
		  delayLeft[delayPtr] = srcLeft[i] + delayLeft[delayPtr] * eState.feedback;
		  delayRight[delayPtr] = srcRight[i] + delayRight[delayPtr] * eState.feedback;
		  delayPtr = (delayPtr + 1) % eState.delayLen;
		  txBuf[pos] = (lval>>16)&0xFFFF;
		  txBuf[pos+1] = lval&0xFFFF;
		  txBuf[pos+2] = (rval>>16)&0xFFFF;
		  txBuf[pos+3] = rval&0xFFFF;

		  i++;
	  }

}



void EchoUpdateState(){
	int newEncoderVal1 = TIM1->CNT;
	int encoderDelata1 = newEncoderVal1-eState.lastEncoderVal1;
	int newEncoderVal2 = TIM2->CNT;
	int encoderDelata2 = newEncoderVal2-eState.lastEncoderVal2;

	//encoder1:
	if(encoderDelata1 == 0){
		goto encoder2;
	}
	if (abs(encoderDelata1) > 20 ) {
		eState.lastEncoderVal1 = newEncoderVal1;
		goto encoder2;
	}
	GPIO_PinState encSW = HAL_GPIO_ReadPin(EncoderSW_GPIO_Port, EncoderSW_Pin);
	if(eState.currentParam1 == 0){
		if(encSW == GPIO_PIN_RESET){
			eState.currentParam1 = 1;
			eState.lastEncoderVal1 = newEncoderVal1;
			goto encoder2;
		}
		eState.delayLen += (encoderDelata1)*200;
		//dState.delayLen = dState.delayLen % BUF_SAMPLES;
		if(eState.delayLen >= DELAY_BUF){
			eState.delayLen = DELAY_BUF -1;
		}
		if(eState.delayLen < 1){
			eState.delayLen = 1;
		}
	}
	else if(eState.currentParam1 == 1){
		if(encSW == GPIO_PIN_SET){
			eState.currentParam1 = 0;
			eState.lastEncoderVal1 = newEncoderVal1;
			goto encoder2;
		}
		eState.mix += ((float)(encoderDelata1))*0.01;
		if(eState.mix < 0.) eState.mix = 0.;
		if(eState.mix > 1.) eState.mix = 1.;
	}

	encoder2:
	if(encoderDelata2 == 0){
			goto updateEnd;
	}
	if (abs(encoderDelata2) > 20) {
			eState.lastEncoderVal2 = newEncoderVal2;
			goto updateEnd;
	}
	eState.feedback += ((float)encoderDelata2)*0.01;
	if(eState.feedback < 0.) eState.feedback = 0.;
	if(eState.feedback > 1.) eState.feedback = 1.;

	updateEnd:
	if(eState.lastEncoderVal1 != newEncoderVal1 || eState.lastEncoderVal2 != newEncoderVal2){
		EchoPrintState();
	}
	eState.lastEncoderVal1 = newEncoderVal1;
	eState.lastEncoderVal2 = newEncoderVal2;
}

void EchoPrintInit(){
	lcd_clear(lcdPtr);
	lcd_gotoxy(lcdPtr, 0, 0);
	lcd_puts(lcdPtr, "Echo Len smp:");

	lcd_gotoxy(lcdPtr, 0, 1);
	lcd_puts(lcdPtr, "Echo Feedback%:");

	lcd_gotoxy(lcdPtr, 0, 2);
	lcd_puts(lcdPtr, "Echo Wet%:");

	EchoPrintState();
}

void EchoPrintState(){

	sprintf(strBuff, "%d     ", eState.delayLen);
	strBuff[7] = '\0';
	lcd_gotoxy(lcdPtr, 13, 0);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d  ", (int) (eState.feedback*100.));
	lcd_gotoxy(lcdPtr, 15, 1);
	lcd_puts(lcdPtr, strBuff);
	sprintf(strBuff, "%d  " , (int)(eState.mix*100.));
	lcd_gotoxy(lcdPtr, 10, 2);
	lcd_puts(lcdPtr, strBuff);


}
