/*
 * LPD6803RFShowControl.cpp
 * This class is a wrappter of the Adafruit LPD6803 pixel library.
 *
 *  Created on: May 28, 2013
 *      Author: Greg Scull
 */

#include <TimerOne.h>

#include "LPD6803RFShowControl.h"

LPD6803RFShowControl::LPD6803RFShowControl(uint8_t pClock, uint8_t pData) : LPD6803(50,pClock,pData)
{
  this->LPD6803::begin();
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void LPD6803RFShowControl::Paint(void)
{
  this->show();
  //delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed??
}
