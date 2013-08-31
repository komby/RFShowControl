/*
* RenardReceiver.ino
*
*  Created on: Mar  2013
*      Author: Materdaddy
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
#include <EEPROM.h>
#include "printf.h"


/*************************** CONFIGURATION SECTION *************************************************/

// Define a Unique receiver ID.  This id should be unique for each receiver in your setup.
// If you are not using Over The air Configuration you do not need to change this setting.
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values:  MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF, RFCOLOR2_4
#define NRF_TYPE			RF1_1_3

//What baud rate does your Renard board you are controlling need?
#define RENARD_BAUD_RATE		57600

// Set OVER_THE_AIR_CONFIG_ENABLEG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for a short period after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

// If you're not using Over-The-Air configuration these variables are required:
#define START_CHANNEL 		1
#define NUM_CHANNELS 		64


//What RF Channel do you want to listen on?
//Valid Values: 1-124
#define LISTEN_CHANNEL 100

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

/***************************  END CONFIGURATION SECTION *************************************************/
// OPTIONAL VARIABLES
#define DEBUG // Uncomment this line to enable debugging

#define PIXEL_TYPE			RENARD
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>


void setup()
{
	Serial.begin(RENARD_BAUD_RATE);
	#ifdef DEBUG
	printf_begin();
	#endif

	Serial.println("Initializing Radio");

	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	
	uint8_t logicalControllerNumber = 0;
	#if (!OVER_THE_AIR_CONFIG_ENABLE )
	
	radio.AddLogicalController(logicalControllerNumber++, START_CHANNEL, NUM_CHANNELS, RENARD_BAUD_RATE);
	#endif

	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);

	radio.printDetails();

	logicalControllerNumber = 0;
	strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber++));

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

//RF Listening for data
void loop(void)
{
	
	//When Radio.Listen returns true its time to update the LEDs for all controllers,  A full update was made
	if (radio.Listen() )
	{
		strip.Paint();
	}
}