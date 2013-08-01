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
	printf("WM2999PixelControl::Paint\n");
	this->WM2999::paint();
}

void WM2999PixelControl::Begin(uint8_t* pDataPointer, int pNumLEDs)
{
	this->externalDataPointerSet = true;
	this->IPixelControl::pixels = pDataPointer;
	this->WM2999::pixels = pDataPointer;
	this->numLEDs  = pNumLEDs;
	this->numberOfPixels = pNumLEDs;   //number of pixels
	
}
void WM2999PixelControl::SetDataBasePointer( uint8_t* dataPointer)
{
	this->IPixelControl::pixels = dataPointer;
	this->WM2999::pixels = dataPointer;
	this->externalDataPointerSet = true;

}