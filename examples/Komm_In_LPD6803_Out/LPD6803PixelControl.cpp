/*
 * LPD6803PixelControl.cpp
 * This class is a wrappter of the Adafruit LPD6803 pixel library.
 * 
 *  Created on: May 28, 2013
 *      Author: Greg Scull
 */

#include "LPD6803PixelControl.h"
#include <TimerOne.h>

LPD6803PixelControl::LPD6803PixelControl(uint8_t pClock, uint8_t pData) : LPD6803(50,pClock,pData) 
{
	this->LPD6803::begin();
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void LPD6803PixelControl::Paint()
{
	this->show();
	//delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}

void LPD6803PixelControl::SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
	this->setPixelColor(n, b, g, r);
}