/*
 * echo.h
 *
 *  Created on: May 14, 2025
 *      Author: sasha
 */

#include "i2c_lcd.h"
#include "filter_dsp.h"
#include "main.h"

typedef struct {
	float freq;
	float res;
	float mix;
	enum filterType type;
	int filterTypeTracker; //to controll numer of incearcing frames to change  filter type
} filterEffectState;

void doFilterEffect(int m);
void FilterEffectInit(I2C_LCD_HandleTypeDef *lcd, int audioFreq);
void FilterEffectUpdateState(masterState* mState);
void FilterEffectPrintInit();
void FilterEffectPrintState();
