/*
 * LPD6803PixelControl.cpp
 * This class is a wrappter of the Adafruit LPD6803 pixel library.
 * 
 *  Created on: May 28, 2013
 *      Author: Greg Scull
 */

#include "LPD6803PixelControl.h"
#include <TimerOne.h>

LPD6803PixelControl::LPD6803PixelControl() : LPD6803(50, 2, 3)
{
	this->begin();
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void LPD6803PixelControl::Paint()
{
	this->show();
	delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}
