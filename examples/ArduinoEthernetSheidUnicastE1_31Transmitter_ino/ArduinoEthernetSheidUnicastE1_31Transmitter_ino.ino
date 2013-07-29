#include <util.h>
#include <EthernetUdp.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <Dns.h>
#include <Dhcp.h>
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#include "E131Constants.h"
#include <OTAConfig.h>
/*
* ArduinoEthernet Sheild Unicast E1.31 Transmitter
*
* Unicast transmitter for RFPixelControl receivers
* This code can be used to send data to the RF1 series controllers.
*
*  Created on: June 17, 2013
*      Author: Greg Scull
*/


/***************************  CONFIGURATION SECTION *************************************************/
#define NRF_TYPE MINIMALIST_SHIELD

#include <RFPixelControlConfig.h>

//How many pixels do you want to transmit data for
//#define NUM_CHANNELS 512
#define RF_DELAY      2000
#define NUM_CHANNELS 220
//use channel 100
#define TRANSMIT_CHANNEL 100
#define DATA_RATE RF24_250KBPS
//Setup  a RF pixel control
//RF1 v.01 board uses Radio 9,10
//ARDUINO ETHERENET NEEDS PIN 10 Relocate CSN to pin8
//RFPixelControl radio(9,8);

//kombyone due transmitter board radio settings.
//RFPixelControl radio(33,10);

/***************************  CONFIGURATION SECTION *************************************************/

//change the mac (0xEF, 0xE0, 0xE1, ... 0xE9)
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change this IP to one on your network
IPAddress ip(192, 168, 2, 76);


//
//// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
#define RF_NUM_PACKETS 18


//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START  126//DMX Packet Position 0xA8


char packetBuffer[700]; //buffer to hold incoming packet,
byte str[32];
EthernetUDP Udp;

void setup(){
	delay(3);
	uint16_t PortNr = 5568;

	Ethernet.begin(mac,ip);
	delay(3);
	Udp.begin(PortNr);
	delay(3);
	Serial.begin(57600);
	Serial.println("\n[E1.31 Arduino Ethernet Transmitter ]\n");
	printf_begin();
	delay(3);
	radio.Initialize( 1, pipes, TRANSMIT_CHANNEL, DATA_RATE, 0);
	//radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, 0);
	delayMicroseconds(1500);

	radio.printDetails();
	delay(3);
}
int maxprint=0;
int len=0;
int freecount=0;
long duration = millis();
long durationAfter = 0;
long checkduration = 0;
boolean status;
void loop () {
	int packetSize = Udp.parsePacket();

	
	Udp.read(packetBuffer,700);
	if (
	packetBuffer[RLP_PREAMBLE_SIZE] == RLP_PREAMBLE_SIZE_VALID
	&& packetBuffer[RLP_POST_AMBLE_SIZE] == RLP_POST_AMBLE_SIZE_VALID
	// && packetBuffer[ACN_PACKET_IDENTIFIER] == need to verify ID
	&& packetBuffer[IDX_VECTOR_PROTOCOL_H] == E1_31_VECTOR_VAL_H
	&& packetBuffer[IDX_VECTOR_PROTOCOL_L] == E1_31_VECTOR_VAL_L
	//
	//&& packetBuffer[] ==
	//&& packetBuffer[] ==
	//&& packetBuffer[] ==
	//&& packetBuffer[] ==
	//&& packetBuffer[] ==
	
	
	&& packetBuffer[E1_31_DMP_ADDRESS_TYPE_DATA_TYPE] == E1_31_DMP_ADDRESS_TYPE_DATA_TYPE_VALID
	&& packetBuffer[E1_31_DMP_FIRST_PROP_ADDR] == E1_31_DMP_FIRST_PROP_ADDR_VALID
	&& packetBuffer[E1_31_FRAMING_VECTOR] == E1_31_FRAMING_VECTOR_VALID )
	{
		
		
		int channelPos=0;
		int i = 0;
		int pktCnt=0;
		int r;
		for ( r=0; r< RF_NUM_PACKETS-1; r++){
			memcpy ( &str[0], &packetBuffer[r*30 + DMX_CHANNEL_DATA_START], 30);
			str[30]=r;
			radio.write_payload( &str[0], 32 );
			delayMicroseconds(RF_DELAY);
			status = radio.get_status();
			while (status & 0x01) {
				status = radio.get_status();
			}
		}
		//final packet if its a partial packet
		int test = NUM_CHANNELS - ( r * 30 );
		if (test >0){
			memcpy ( &str[0], &packetBuffer[r*30], test);
			str[30]=r;
			radio.write_payload( &str[0], 32 );
			delayMicroseconds(RF_DELAY);
			status = radio.get_status();
			while (status & 0x01) {
				status = radio.get_status();
			}
		}
	}
	
}