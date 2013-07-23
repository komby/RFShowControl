/*
 * RenardControl.cpp
 * This class is a wrappter of the GEColorEffects library for an IPixelControl implementation.
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
//	delayMicroseconds(200);
}


void RenardControl::Paint()
{
//	digitalWrite(A0, HIGH);
//	delay(5);

	Serial.write(0x7E);
	Serial.write(0x80);

	for ( int i = 0; i < (numLEDs*3); i++ )
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

//	digitalWrite(A0, LOW);
//	delayMicroseconds(200);
}
