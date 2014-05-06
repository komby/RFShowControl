/*
 * RenardControl.cpp
 * This class handles the RFPixelControl library for an IPixelControl implementation.
 * 
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 */

#include "RenardControl.h"

RenardControl::RenardControl(uint32_t baud_rate)
{
	Serial.begin(baud_rate);
	pinMode(A0, OUTPUT);
	digitalWrite(A0, HIGH);
}


void RenardControl::Paint()
{
	Serial.write(0x7E);
	Serial.write(0x80);

	for ( int i = 0; i < (numLEDs); i++ )
	{
		switch (pixels[i])
		{
			case 0x7D:
				Serial.write(0x7F);
				Serial.write(0x2F);
				break;

			case 0x7E:
				Serial.write(0x7F);
				Serial.write(0x30);
				break;

			case 0x7F:
				Serial.write(0x7F);
				Serial.write(0x31);
				break;

			default:
				Serial.write(pixels[i]);
				break;
		}
	}
}
