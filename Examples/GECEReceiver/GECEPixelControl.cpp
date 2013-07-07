/*
 * GECEPixelControl.cpp
 * This class is a wrappter of the GEColorEffects library for an IPixelControl implementation.
 * 
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 */

#include "GECEPixelControl.h"

GECEPixelControl::GECEPixelControl() : GEColorEffects(50,2)
{
}


//GECEPixelControl::begin(uint8_t* pDataPointer, int pNumLEDs)
//{
	//IPixelControl::begin(pDataPointer, pNumLEDs)	;
//}
//
/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void GECEPixelControl::Paint()
{
	cli();

	for ( int i = 0; i < numLEDs; i++ )
	{
		this->set_color(i,
						DEFAULT_INTENSITY,
						this->color((pixels[(i*3)])>>4,
									(pixels[(i*3)+1])>>4,
									pixels[(i*3)+2]>>4
									));
	}

	sei();
}


