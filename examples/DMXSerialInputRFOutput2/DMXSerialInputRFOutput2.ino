/*
* SerialDMXToRFTransmitter
*
*  Created on: Mar  2014
*      Author: Greg Scull, komby@komby.com
*
*     This code is a derivative of the original work done by
*      Joe Johnson RFColor_24 Receiver Program
*
*      The Code which Joe wrote inspired this software, the related hardware and
*      it was also used as a starting point for this class.
*
*      As with the RFColor_24 The commercial Use of this software is Prohibited.
*      Use this software and (especially) the hardware at your own risk.
*   
*      This code also uses a modified version of the DMXSerial library @see ModifiedDMXSerial for more information.
* 
*      
*      Users of this software agree to hold harmless the creators and contributors
*      of this software.  By using this software you agree that you are doing so at your own risk
*
*      NOTE: This no longer requries the use of the hardware serial hack
*/
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>


//#define SCOPEDEBUG 1
#include "ModifiedDMXSerial.h"
/***************************  BEGIN CONFIGURATION SECTION *************************************************/
#define DMX_NUM_CHANNELS 512  // This must be divisible by 3!  This defines the number of DMX channels we are going to listen to.
#define RF_NUM_PACKETS 18     // This is DMX_NUM_CHANNELS divided by 30 rounded up.

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
//Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
#define NRF_TYPE			RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to transmit on?
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 80
/***************************  END CONFIGURATION SECTION *************************************************/

//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>


void setup()
{

	radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL,DATA_RATE ,0);
	ModifiedDMXSerial.maxChannel(DMX_NUM_CHANNELS);
	ModifiedDMXSerial.init(DMXReceiver);
}


void loop()
{
	if (ModifiedDMXSerial.isPacketReady() )
	{
		radio.write_payload( ModifiedDMXSerial.GetPacketPointer(), 32 );
		ModifiedDMXSerial.setPacketReady(false);
	}
}

