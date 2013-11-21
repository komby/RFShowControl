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
#include <MemoryFree.h>
#include <EEPROM.h>
/*
* ArduinoEthernet Sheild Unicast E1.31 Transmitter
*
* Unicast transmitter for RFPixelControl receivers
* This code can be used to send data to the RF1 series controllers.
*
*  Created on: June 17, 2013
*  Updated: October 2013
*      Author: Greg Scull
*/


/***************************  CONFIGURATION SECTION *************************************************/

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
//Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
#define NRF_TYPE			MINIMALIST_SHIELD

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to transmit on?
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 100


//Each ethernet device needs its own MAC address
//You can get your own mac string for this at
//http://www.miniwebtool.com/mac-address-generator/
byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change this IP to one on your network
IPAddress ip(192, 168, 2, 151);

//This is a rate in milliseconds which the transmitter will resend all the data if no data is received within this duration.  
//if this setting is set to 0 it will disable the keepalive
#define REFRESH_RATE 0

//This is the universe you want to receive with this transmitter. 
//you can receive Multicast OR unicast with this setting
#define UNIVERSE 2

/***************************  CONFIGURATION SECTION *************************************************/
//Changed to not use the RFPixelControl Config 
//and instead use the RF24Wrapper directly to save space
//this can and should be reafactored eventually.

#define RF_NUM_PACKETS 18
// Radio pipe
// addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
	0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};

#if (NRF_TYPE == MINIMALIST_SHIELD)
RF24Wrapper radio(9, 8);
#elif (NRF_TYPE == RF1_1_2) || \
(NRF_TYPE == RF1_1_3) || \
(NRF_TYPE == RF1_0_2) || \
(NRF_TYPE == RF1_12V_0_1) ||\
(NRF_TYPE == KOMBEE)
RF24Wrapper radio(8, 7);
#elif (NRF_TYPE == KOMBYONE_DUE)
RF24Wrapper radio(33,10);
#elif (NRF_TYPE == WM_2999_NRF) || \
(NRF_TYPE == RFCOLOR2_4)
RF24Wrapper radio(9,10);
#else
#error Must define an NRF type!
#endif



//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START  126//DMX Packet Position 0xA8
#define DMX_MAX_CHANNEL  512


uint8_t buffer1[638]; //buffer to hold incoming packet,
uint8_t buffer2[638];  //buffer to cache good universe packet 
uint8_t* buf;
uint8_t* validBuf;


byte str[32];
EthernetUDP Udp;


void setup(){

	uint16_t PortNr = 5568;
	Ethernet.begin(mac,ip);
	Udp.begin(PortNr);
	
	Serial.begin(57600);
	Serial.println(F("\n[E1.31 Arduino Ethernet Transmitter ]\n"));
	printf_begin();
	
	Serial.print(F("freeMemory()="));
	Serial.println(freeMemory());
	radio.Initialize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE);
	delayMicroseconds(1500);

	radio.printDetails();
}
int maxprint=0;
int len=0;
int freecount=0;
long duration = millis();
uint16_t numChannelsInPacket;
uint8_t numPackets;
uint16_t universe;
int _rfinterpacketdelay = DATA_RATE == RF24_1MBPS ? 700 : 1500;
boolean status;
void loop () {
	int packetSize = Udp.parsePacket();
	if ( packetSize > 0 )
	{
		if ( validBuf == buffer2 ){
			buf=buffer1;
			
		}
		else
			buf=buffer2; 
			
		Udp.read(buf, 638);
					
		const uint64_t pPipes[2] =
		{
			0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
		};

		bool validateR = validateRoot(buf);
		bool vdmp = validateDMP(buf);
		bool vfr = validateFraming(buf);

		if( validateR && vdmp && vfr)
		{
			duration = millis();
			
		
			int channelPos=0;
     
			numChannelsInPacket = ( (buf[E1_31_DMP_PROP_VAL_CNT] <<8)  | buf[E1_31_DMP_PROP_VAL_CNT+1] ) -1 ;
			numPackets = numChannelsInPacket / 30;
			universe = ((buf[E1_31_FRAMING_UNIVERSE_ID] <<8) | buf[E1_31_FRAMING_UNIVERSE_ID+1]);
		
				transmitDataFromBuffer(buf);
		}
	}
	else if(REFRESH_RATE && millis() - duration >= REFRESH_RATE)
	{
		//no data received re-send
		transmitDataFromBuffer(validBuf);
		duration = millis();
	}
}

void transmitDataFromBuffer(uint8_t* pBuffer)
{

		int r;

			if (numChannelsInPacket <=DMX_MAX_CHANNEL && numChannelsInPacket >0 && numPackets <= 18 && universe == UNIVERSE)
			{
				validBuf = pBuffer;
				for ( r=0; r< numPackets; r++){
				
					memcpy ( &str[0], &validBuf[r*30 + E1_31_DMP_FIRST], 30);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
					delayMicroseconds(_rfinterpacketdelay);

				}
				//final packet if its a partial packet
				int test = numChannelsInPacket - ( (r) * 30 );

				if (test >0){
					memcpy ( &str[0], &validBuf[(r)*30 + E1_31_DMP_FIRST], test);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
					delayMicroseconds(_rfinterpacketdelay);
				}
			}
}

bool validateDMP(uint8_t buf[]) {
	if (buf[E1_31_DMP_VECTOR] != E1_31_DMP_VECTOR_VALID
	|| buf[E1_31_DMP_ADDRESS_TYPE_DATA_TYPE] != E1_31_DMP_ADDRESS_TYPE_DATA_TYPE_VALID
	|| ntos(buf + E1_31_DMP_FIRST_PROP_ADDR) != E1_31_DMP_FIRST_PROP_ADDR_VALID
	|| ntos(buf + E1_31_DMP_ADDRESS_INCREMENT) != E1_31_DMP_ADDRESS_INCREMENT_VALID)
	return false;
	//         ntos(buf + E1_31_DMP_PROP_VAL_CNT) - 1;
	return true;
}

bool validateFraming (uint8_t buf[])
{

	if (ntoi(buf + E1_31_FRAMING_VECTOR) != E1_31_FRAMING_VECTOR_VALID)
	return false;
	return true;
}

bool validateRoot (uint8_t buf[])
{

	if (ntos(buf) != RLP_PREAMBLE_SIZE_VALID
	|| ntos(buf + RLP_POST_AMBLE_SIZE) != RLP_POST_AMBLE_SIZE_VALID
	|| ntoi(buf + IDX_VECTOR_PROTOCOL) != 0x00000004)
	return false;
	return true;
}


