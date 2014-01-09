/*
 * GWTSControl.cpp
 * This class is a wrapper of the GwtS library for an IPixelControl implementation.
 * 
 *  Created on: November 25th, 2013
 *      Author: Mat Mrosko
 */

#include <GWTSControl.h>

GWTSControl::GWTSControl() : GWTS()
{
}


/**
 * Paint method to keep compatibility with the other Pixel types
 * 
 */
void GWTSControl::Paint()
{
	this->set_colors(pixels[0],
					 pixels[1],
					 pixels[2],
					 pixels[3],
					 pixels[4],
					 pixels[5]);
}
