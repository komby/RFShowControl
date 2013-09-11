/*
 * IPixelControl.cpp
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg Scull/Mat Mrosko
 */

#include "IPixelControl.h"


IPixelControl::IPixelControl()
{
	//numLEDs = ((pixels = (uint8_t *)calloc(50, 3)) != NULL) ? 50 : 0;
	dataPin=2;
}



IPixelControl::~IPixelControl()
{
 	if (pixels != NULL)
	free(pixels);
}

void IPixelControl::Begin(uint8_t* pDataPointer, int pNumLEDs)
{
	this->pixels = pDataPointer;
	this->numLEDs  = pNumLEDs;
}	



uint16_t IPixelControl::GetPixelCount( void )
{
	return numLEDs;
}

void IPixelControl::SetPixelCount( uint16_t count )
{
 	if (pixels != NULL)
		free(pixels);
	numLEDs = ((pixels = (uint8_t *)calloc(count, 3)) != NULL) ? count : 0;
}

uint32_t IPixelControl::GetPixelColor(uint16_t n)
{
	if (n < numLEDs)
	{
		uint16_t ofs = n * 3;
		// To keep the show() loop as simple & fast as possible, the
		// internal color representation is native to different pixel
		// types.  For compatibility with existing code, 'packed' RGB
		// values passed in or out are always 0xRRGGBB order.
		return ((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2];
	}

	return 0; // Pixel # is out of bounds
}

void IPixelControl::SetPixelColor(uint16_t n, uint32_t c, uint8_t pColorOrder)
{
	if (n < numLEDs)
	{
		
		uint8_t *p = &pixels[n * 3];
		  if (pColorOrder == RGB_ORDER ) 
		{
			// To keep the show() loop as simple & fast as possible, the
			// internal color representation is native to different pixel
			// types.  For compatibility with existing code, 'packed' RGB
			// values passed in or out are always 0xRRGGBB order.
			*p++ = c >> 16; // Red
			*p++ = c >>  8; // Green
			*p++ = c;       // Blue
		}
		else if (pColorOrder == BGR_ORDER )
		{
			*p++ = c;		// Red
			*p++ = c >>  8; // Green
			*p++ = c >>  16;// Blue
		} 			
	}
}

void IPixelControl::SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t pColorOrder)
{
	if(n < numLEDs) // Arrays are 0-indexed, thus NOT '<='
	{
		uint8_t *p = &pixels[n * 3];
	    if (pColorOrder == RGB_ORDER ) 
		{
			
			// See notes later regarding color order
			*p++ = r;
			*p++ = g;
			*p++ = b;
		}	
		else if (pColorOrder == BGR_ORDER )  
		{
			*p++ = b;
			*p++ = g;
			*p++ = r;
		}				
	}
}

uint32_t IPixelControl::Color(byte r, byte g, byte b)
{
	uint32_t c;
	c = r;
	c <<= 8;
	c |= g;
	c <<= 8;
	c |= b;
	return c;
}

void IPixelControl::SetDataBasePointer( uint8_t* dataPointer)
{
	this->pixels = dataPointer;

}