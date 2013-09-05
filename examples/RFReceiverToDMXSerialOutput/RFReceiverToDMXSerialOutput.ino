

/*
* DMXSerial  RFPixel Control Receiver Sketch for handling the RF to DMX
*
*  Created on: August 2013
*      Author: Greg Scull, komby@komby.com
*
*/


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
//This uses a modified version of the DMXSerial library.
//in order to work with the
#include "ModifiedDMXSerial.h"


/**************CONFIGURATION SECTION ***************************/
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup.
// If you are not using Over The air Configuration you do not need to change this setting.
//Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values: RFCOLOR2_4, MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF
#define NRF_TYPE  RF1_1_3

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

// If you're not using Over-The-Air configuration these variables are required:
//If you are using OTA then your done,  ignore this stuff.
#define HARDCODED_NUM_DMX_CHANNELS 512
#define HARDCODED_START_CHANNEL 1
//What RF Channel do you want to listen on?
//Valid Values: 1-124
#define LISTEN_CHANNEL 100


/**************END CONFIGURATION SECTION ***************************/
#define RECEIVER_NODE 1
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>
uint8_t* channels;
//#define SCOPEDEBUG
void setup() {
	//NOTE You CANNOT use any Serial.Write with this sketch
	
	//Set the output pin for the RS485 adapter to transmit
	pinMode(A0, OUTPUT);
	digitalWrite(A0, HIGH);
	
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	
	uint8_t logicalControllerNumber = 0;
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		
		radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_DMX_CHANNELS,  0);
	}
	
	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
	
	logicalControllerNumber = 0;
	
	channels =radio.GetControllerDataBase(logicalControllerNumber);
	int numChannels = radio.GetNumberOfChannels(logicalControllerNumber);
	ModifiedDMXSerial.maxChannel(numChannels);

	ModifiedDMXSerial.init(DMXController,channels);

}

void loop() {
	//we dont need to do anything here as the library is handling all the data and transmission of info.
	if (radio.Listen() )
	{
		
	}

}