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
#include <EEPROM.h>
/*
* ArduinoEthernet Sheild Unicast E1.31 Transmitter
*
* Unicast transmitter for RFPixelControl receivers
* This code can be used to send data to the RF1 series controllers.
*
*  Created on: June 17, 2013
*  Updated: July, 2013
*      Author: Greg Scull
*/


/***************************  CONFIGURATION SECTION *************************************************/

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE			MINIMALIST_SHIELD

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_1MBPS

//What RF Channel do you want to transmit on?
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 80


//Each ethernet device needs its own MAC address
//You can get your own mac string for this at
//http://www.miniwebtool.com/mac-address-generator/
byte mac[] = {
0x6B, 0x88, 0x9E, 0x82, 0x32, 0x6C };

//Change this IP to one on your network
IPAddress ip(192, 168, 2, 157);


/***************************  CONFIGURATION SECTION *************************************************/
#include <RFPixelControlConfig.h>
#define RF_NUM_PACKETS 18
#define RF_DELAY      500



#define UNIVERSE_1 7
#define UNIVERSE_2 8


#define UNIVERSE_1_RF 70
#define UNIVERSE_2_RF 80



//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START  126//DMX Packet Position 0xA8
#define DMX_MAX_CHANNEL  512


uint8_t packetBuffer[638]; //buffer to hold incoming packet,
byte str[32];
EthernetUDP Udp;
int universes[4] = {UNIVERSE_1, UNIVERSE_2};
int universe_RF[4] = {UNIVERSE_1_RF, UNIVERSE_2_RF};

void setup(){

	uint16_t PortNr = 5568;
	Ethernet.begin(mac,ip);
	Udp.begin(PortNr);

	Serial.begin(57600);
	Serial.println("\n[E1.31 Arduino Ethernet Transmitter ]\n");
	printf_begin();

	radio.Initialize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, 0);
	delayMicroseconds(1500);

	radio.printDetails();
}
int maxprint=0;
int len=0;
int freecount=0;
long duration = millis();
long durationAfter = 0;
long checkduration = 0;
boolean status;
void loop () {
  bool validUniverse = true;
	int packetSize = Udp.parsePacket();
	if ( packetSize > 0 ){
		Udp.read(packetBuffer,638);
		const uint64_t pPipes[2] = {
			0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
		};
uint16_t universe = ((packetBuffer[E1_31_FRAMING_UNIVERSE_ID] <<8) | packetBuffer[E1_31_FRAMING_UNIVERSE_ID+1]);

			//make sure we are transmitting on the correct channel.
			if (universe == UNIVERSE_1 )
			{
          
			       radio.ChangeTransmitChannel(UNIVERSE_1_RF);
				//radio.printDetails();

			}
			else if ( universe == UNIVERSE_2  )
			{
				//while( radio.
				radio.ChangeTransmitChannel(UNIVERSE_2_RF);
				//radio.printDetails();
			}
			

		bool validateR = validateRoot(packetBuffer);
		bool vdmp = validateDMP(packetBuffer);
		bool vfr = validateFraming(packetBuffer);
		//printf("ts");
		if( validateR && vdmp && vfr && validUniverse)
		{
			//printf("t");
			
			int channelPos=0;
			int i = 0;
			int pktCnt=0;
			int r;
			uint16_t numChannelsInPacket = ( (packetBuffer[E1_31_DMP_PROP_VAL_CNT] <<8)  | packetBuffer[E1_31_DMP_PROP_VAL_CNT+1] ) -1 ;
			uint8_t numPackets = numChannelsInPacket / 30;
			

			if (numChannelsInPacket <=DMX_MAX_CHANNEL && numChannelsInPacket >0 && numPackets <= 18 )
			{

				for ( r=0; r< numPackets; r++){
					//printf("outputdatas %d\n", radio.GetChannel());
					memcpy ( &str[0], &packetBuffer[r*30 + E1_31_DMP_FIRST], 30);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
					delayMicroseconds(RF_DELAY);

				}
				//final packet if its a partial packet
				int test = numChannelsInPacket - ( (r) * 30 );

				if (test >0){
					memcpy ( &str[0], &packetBuffer[(r)*30 + E1_31_DMP_FIRST], test);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
delayMicroseconds(RF_DELAY);
				}
			}
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

