/*

Modified from Jon's original work to compile with current RFShow Control code
opyright (C) 2013 Jon Chuchla <jon@chuchla.com>

Modified from original work by:
Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
*/

/* CHANNEL INTERFERENCE SCANNER
*
* Example to detect interference on the various channels available.
* This is a good diagnostic tool to check whether you're picking a
* good channel for your application.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <printf.h>
// Set up nRF24L01 radio on SPI bus plus pins
//RF24 radio(9, 8);  //set up for Komby Minimalist shield on arduino Uno R3

const uint8_t num_channels = 128;
uint8_t values[num_channels];
bool RadioIsPlus;



/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1
/********************** END OF REQUIRED CONFIGURATION ************************/


/************** START OF ADVANCED SETTINGS SECTION DO NOT MODIFY****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
#define DEBUG   1
//FCC_RESTRICT Description: http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT
//Valid Values: 1, 0  (1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1


// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

#define PIXEL_TYPE                      NONE
/************** END OF ADVANCED SETTINGS SECTION DO NOT MODIFY****************/


//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

void setup(void)
{
	// Print preamble
	Serial.begin(115200);
	printf_begin();
	printf("\nnRF24L01 Interference Scanner\n\r");
	printf("(c)2013, Jon Chuchla\n\r");
	printf("based on original 2011 work by J. Coliz\n\n");
	
	// Setup and configure rf radio
	radio.begin();
	radio.setAutoAck(false);

	//Test for Plus Version
	RadioIsPlus = radio.isPVariant();
	if (RadioIsPlus)
		printf("Connected Radio is nRF24L01+ (Plus Version)\n");
	else printf("Connected Radio is nRF24L01 (non Plus version)\n");

	// Get into standby mode
	radio.startListening();
	radio.stopListening();

	// Print out header, Plain text
	int i = 0;
	printf("\n\r");
	while (i < num_channels)
	{
		printf("%i,", i);
		++i;
	}
	printf("\n\r");
}

const int num_reps = 100;

void loop(void)
{
	// Clear Previous measurement values
	memset(values, 0, sizeof(values));

	// Scan all channels num_reps times
	int rep_counter = num_reps;
	while (rep_counter--)
	{
		int i = num_channels;
		while (i--)
		{
			// Select this channel
			radio.setChannel(i);

			// Listen for a little
			radio.startListening();
			delayMicroseconds(128);
			radio.stopListening();

			// Did we get anything this time?
			if(RadioIsPlus)
			{
				// Did we get a signal above -64dBm?
				if (radio.testRPD())
					++values[i];
			}
			else
			{
				// Did we get a carrier?
				if (radio.testCarrier())
					++values[i];
			}
		}
	}

	// Print out channel measurements, clamped to a single hex digit
	int i = 0;
	while (i < num_channels)
	{
		printf("%x,", min(0xf, values[i] & 0xf));
		++i;
	}
	printf("\n\r");
}