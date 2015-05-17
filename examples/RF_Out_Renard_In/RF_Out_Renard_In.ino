/*
* Renard Transmitter
*
* TODO
*
*  Author: Greg Scull
*      7/2013
*   This transmitter uses
*/

#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

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

// RENARD_BAUD_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#RENARD_BAUD_RATE
// Valid Values: 19200, 38400, 57600, 115200, 230400, 460800
#define RENARD_BAUD_RATE                57600
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
#define RF_WRAPPER                      1
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

#define RF_NUM_PACKETS                  18     // 18 * 30 total channels can be broadcasted
#define RECEIVER_UNIQUE_ID              0
#define PAD                             0x7D
#define SYNC                            0x7E
#define ESCAPE                          0x7F
#define COMMAND                         0x80

volatile unsigned int sub1 = 0;
volatile bool packetready = false;

//Initialize the RF packet buffer
byte str[RF_NUM_PACKETS][32];


void setup(void)
{
	radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);
	UCSR0C |= (1 << USBS0);
	Serial.begin(RENARD_BAUD_RATE);
}

void loop(void)
{
	if (packetready)
		//If there is a packet ready to write to the radio...Write it!
	{
		packetready = false;
		radio.writeFast(&str[sub1 - 1], 32);
	}
}