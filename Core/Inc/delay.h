/*
 * delay.h
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */

#ifndef SRC_DELAY_H_
#define SRC_DELAY_H_
#include "i2c_lcd.h"
#endif /* SRC_DELAY_H_ */


void doDelay( int m );
void DelayInit(I2C_LCD_HandleTypeDef* lcd);
void DelayUpdateState();
void DelayPrintInit();
void DelayPrintState();

typedef struct delayState {
	int delayLen;
	float mix;
	int currentParam;
	int lastEncoderVal;
}delayState;

