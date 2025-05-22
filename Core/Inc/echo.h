/*
 * echo.h
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */

#ifndef SRC_ECHO_H_
#define SRC_ECHO_H_
#include "i2c_lcd.h"

#include "main.h"

#endif /* SRC_DELAY_H_ */


void doEcho( int m );
void EchoInit(I2C_LCD_HandleTypeDef* lcd, int audioFreq);
void EchoUpdateState(masterState* mState);
void EchoPrintInit();
void EchoPrintState();

typedef struct {
	int delayLen;
	float mix;
	float feedback;
	float filterFreq;
}echoState;

