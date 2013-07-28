/*
* Test Transmitter sketch.
*
* Generates a continuous sequence of RF data as a transmitter for debugging purposes.
*
*  Created on: Mar 25, 2013
*  Author: Greg Scull
*/

#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/***************************  CONFIGURATION SECTION BEGIN *************************************************/

//How many pixels do you want to transmit data for (NOTE: Pixels, not channels)
#define NUM_PIXELS 170

//Delay between RF transmissions (in microseconds)
#define RF_DELAY      2000

#define DATA_RATE RF24_1MBPS
//Use channel 100
#define TRANSMIT_CHANNEL 100

#define NRF_TYPE MINIMALIST_SHIELD
//#define NRF_TYPE RF1_1_3


/***************************  END CONFIGURATION SECTION BEGIN *************************************************/
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>

//Initialize the RF packet buffer
byte str[32];

const int numberOfChannels = NUM_PIXELS * 3;

byte buffer[numberOfChannels];

void setup()
{
	Serial.begin(57600);
	Serial.println("\n[TEST RF Transmitter]\n");
	printf_begin();
	delay(5);
	radio.Initalize( 1, pipes, TRANSMIT_CHANNEL , DATA_RATE , 0);
	
	delay(5);
	radio.printDetails();
	delay(5);
}


void loop ()
{
	
	rainbow( NUM_PIXELS);
}


// Show update method handles the transmission of the RF packets.  Its using the RF DMX technique used by Joe Johnson for the RFColor2_4 library
void showUpdate()
{
	byte status = 0;

	for(int ii=0, kk=0,jj=0;ii< numberOfChannels && kk<32 ;ii++)
	{
		str[kk++] = buffer[ii];//set the byte color
		if (kk == 30)
		{
			str[kk] = jj++;
			kk=0;
			radio.write_payload( &str[0], 32 );
			delayMicroseconds(RF_DELAY);
			status = radio.get_status();

			while (status & 0x01)
			{
				status = radio.get_status();
			}
		}
	}
}

/*
Helper functions from ADAFRUIT ws8201 library
The MIT License (MIT)

Copyright (c) <year> <copyright holders>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Modified by Greg scull to handle RF transmission.
*/
void rainbow( int num)
{
	int i, j;

	for (j=0; j < 256; j++)     // 3 cycles of all 256 colors in the wheel
	{
		for (i=0; i < num; i++)
		{
			uint32_t c  = Wheel( ((i * 256 / num) + j) % 256);
			uint8_t *p = &buffer[i * 3];
			*p++ = c;         // Blue
			*p++ = c >>  8; // Green
			*p++ = c >> 16; // Red
		}
		showUpdate();
	}
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
	uint32_t c;
	c = r;
	c <<= 8;
	c |= g;
	c <<= 8;
	c |= b;
	return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
	if (WheelPos < 85)
	{
		return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if (WheelPos < 170)
	{
		WheelPos -= 85;
		return Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else
	{
		WheelPos -= 170;
		return Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}