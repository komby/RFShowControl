/*
* SerialDMXToRFTransmitter
*
*    This code is a derivative of the original work done by
*    Joe Johnson RFColor_24 Receiver Program
*
*    The Code which Joe wrote inspired this software, the related hardware and
*    it was also used as a starting point for this class.
*
*    As with the RFColor_24 The commercial Use of this software is Prohibited.
*    Use this software and (especially) the hardware at your own risk.
*
*    This code also uses a modified version of the DMXSerial library @see ModifiedDMXSerial for more information.
*
*    NOTE: This no longer requries the use of the hardware serial hack
*
* Created on: Mar 2014
* Author: Greg Scull, komby@komby.com
*
* License:
*    Users of this software agree to hold harmless the creators and
*    contributors of this software.  By using this software you agree that
*    you are doing so at your own risk, you could kill yourself or someone
*    else by using this software and/or modifying the factory controller.
*    By using this software you are assuming all legal responsibility for
*    the use of the software and any hardware it is used on.
*
*    The Commercial Use of this Software is Prohibited.
*/

#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include <DMXSerial.h>
#include "RFShowControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// TRANSMIT_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#TRANSMIT_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define TRANSMIT_CHANNEL                10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_CHANNELS
// Valid Values: 1-512
#define HARDCODED_NUM_CHANNELS          512
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
//Include this after all configuration variables are set

#include "RFShowControlConfig.h"

bool txstat = 1;
bool initclean = 0;

void setup(void)
{
	pinMode(A0, OUTPUT);
	digitalWrite(A0, HIGH);

	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	initclean = radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, 0);
	DMXSerial.maxChannel(HARDCODED_NUM_CHANNELS);
	DMXSerial.init(DMXReceiver);
	DMXSerial.attachOnUpdate(&SendPackets);
}

void loop(void)
{
	//nothing here.  DMXSerial will call the SendPackets function when a full DMX frame is here.
}

void SendPackets(void)
{
	uint8_t packet[32];
	int chan_cnt = 0;
	int dataBytesInPacket = 30;
	uint8_t *dmxBuffer = DMXSerial.getBuffer();

	uint8_t sizeofbyte = sizeof(uint8_t);
	uint8_t sizeofpacket = sizeofbyte * 30;
	for (int i = 0; i<18 && chan_cnt < HARDCODED_NUM_CHANNELS; i++)
	{
		memcpy(packet, dmxBuffer, sizeofpacket);
		packet[30] = i;
		dmxBuffer += 30;
		radio.writeFast(packet, 32);
		//blink the leds to show we are sending packets
		digitalWrite(1, (txstat && initclean) ? 1 : 0);
		chan_cnt += 30;
	}
}