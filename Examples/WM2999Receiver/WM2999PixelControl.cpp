/*
 * WM2999PixelControl.cpp
 * This class is a wrappter of the Adafruit WM2999 pixel library.
 * 
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#include "WM2999PixelControl.h"

WM2999PixelControl::WM2999PixelControl(uint8_t ppin): WM2999(ppin)
{
	start();
}

void WM2999PixelControl::Paint(void)
{
	paint();
}
