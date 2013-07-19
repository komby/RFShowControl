/*
 * GECERFReceiver.cpp
 *
 *  Created on: Mar  2013
 *      Author: Greg Scull, komby@komby.com
 *      
 *      Users of this software agree to hold harmless the creators and contributors
 *      of this software. You, by using this software, are assuming all legal responsibility
 *      for the use of the software and any hardware it is used on.
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"


//*****************************************************************************

// REQUIRED VARIABLES
#define RECEIVER_UNIQUE_ID 50
#define NRF_TYPE			MINIMALIST_SHIELD
#define PIXEL_TYPE			GECE
#define PIXEL_PIN			3


// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OTA_CONFIG 1


// If you're not using Over-The-Air configuration these variables are required:
#define DMX_START_CHANNEL 0
#define DMX_NUM_PIXELS 50
#define LISTEN_CHANNEL 100	// the channel for the RF Radio


//*****************************************************************************

// OPTIONAL VARIABLES
#define DEBUG // Uncomment this line to enable debugging


//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>



//Arduino setup function.
void setup()
{
	Serial.begin(57600);
#ifdef DEBUG
	printf_begin();
#endif

	Serial.println("Initializing Radio");

	radio.EnableOverTheAirConfiguration(OTA_CONFIG);
#if (OTA_CONFIG == 0)
	radio.AddLogicalController(RECEIVER_UNIQUE_ID, DMX_START_CHANNEL, (DMX_NUM_PIXELS*3), 0);
#endif

	radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL, RF24_250KBPS, RECEIVER_UNIQUE_ID);
	radio.printDetails();
	//radio.PrintControllerConfig();
	
	uint8_t logicalControllerNumber = 0;

	strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber));

	for (int i = 0; i < strip.GetPixelCount(); i++)
		strip.SetPixelColor(i, strip.Color(0, 0, 0));
	strip.Paint();
}

void print_data(char *data)
{
	printf("--%3d:", 0);
	for ( int i = 0; i < strip.GetPixelCount(); i++ )
	{
		printf(" 0x%02X", data[i] & 0xFF);
		if ( (i-7) % 8 == 0 )
		{
			printf("\n\r--%3d:", i);
		}
	}
	printf("\n\r\n");
}

//RF Listening to DMX packets loop
void loop(void)
{
    
    //When Radio.Listen returns true its time to update the LEDs for all controlelrs,  A full update was made
    if (radio.Listen() )
    {
		print_data((char *)radio.GetControllerDataBase(0));
	    strip.Paint();
    }
}
