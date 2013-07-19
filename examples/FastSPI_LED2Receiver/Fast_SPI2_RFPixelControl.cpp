/*
 * Fast_SPI2_RFPixelControl.cpp
 * This class is a wrappter of the Fast_SPI2  ws2801 pixel library.
 * 
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#include "Fast_SPI2_RFPixelControl.h"

void Fast_SPI2_RFPixelControl::InitFastSPI(CLEDController * pController)
{
	//controller is passed in constructor from user.
	if ( pController > 0 )
	{
		this->_fastSPIController=pController;
		this->_fastSPIController->init();
	}
	else
	{
		Serial.print("nullPointer\n");
	}
}
/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void Fast_SPI2_RFPixelControl::Paint(byte * colors , unsigned int count)
{
	this->pixels = colors;
	this->_fastSPIController->showRGB(colors, count);
	delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void Fast_SPI2_RFPixelControl::Paint()
{
	Serial.write("in rfpixelcontrol paint\n");
	this->_fastSPIController->showRGB((byte *)this->pixels, this->numLEDs);
	delay(1); //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}
