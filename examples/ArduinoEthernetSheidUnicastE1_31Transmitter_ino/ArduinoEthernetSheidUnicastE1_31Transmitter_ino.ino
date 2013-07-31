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
#define NRF_TYPE			RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to transmit on?  
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 100

/***************************  CONFIGURATION SECTION *************************************************/
#include <RFPixelControlConfig.h>
#define RF_NUM_PACKETS 18
#define RF_DELAY      2000


//change the mac (0xEF, 0xE0, 0xE1, ... 0xE9)
byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change this IP to one on your network
IPAddress ip(192, 168, 1, 150);




//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START  126//DMX Packet Position 0xA8
#define DMX_MAX_CHANNEL  512


uint8_t packetBuffer[638]; //buffer to hold incoming packet,
byte str[32];
EthernetUDP Udp;

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
	int packetSize = Udp.parsePacket();
	if ( packetSize > 0 ){
		Udp.read(packetBuffer,638);
		printf("packetsize:%d\n",packetSize);
		bool validateR = validateRoot(packetBuffer);
		bool vdmp = validateDMP(packetBuffer);
		bool vfr = validateFraming(packetBuffer);

		//  printf("validate root : %d,  validate DMP:  %d , validateFraming %d\n", validateR,vdmp,vfr);
		if( validateR && vdmp && vfr)
		{
			
			
			int channelPos=0;
			int i = 0;
			int pktCnt=0;
			int r;
			uint16_t numChannelsInPacket = ( (packetBuffer[E1_31_DMP_PROP_VAL_CNT] <<8)  | packetBuffer[E1_31_DMP_PROP_VAL_CNT+1] ) -1 ;
			uint8_t numPackets = numChannelsInPacket / 30;
			//  printf("numPackets %d\n",numPackets);
			if (numChannelsInPacket <=DMX_MAX_CHANNEL && numChannelsInPacket >0 && numPackets <= 18 )
			{
				//printf("numchannels:%d\n",numChannelsInPacket);
				for ( r=0; r< numPackets; r++){
					
					memcpy ( &str[0], &packetBuffer[r*30 + E1_31_DMP_FIRST], 30);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
					delay(1);
					// printf("SentPacket:%d \n\n",r);
					//delayMicroseconds(RF_DELAY);
					
					
					//printf("processingpacketafter:%d\n\n",millis());
				}
				//final packet if its a partial packet
				int test = numChannelsInPacket - ( (r) * 30 );
				
				printf("numChaninpacket %d  -  %d = %d\n",numChannelsInPacket, (r)*30, test);
				if (test >0){
					memcpy ( &str[0], &packetBuffer[(r)*30 + E1_31_DMP_FIRST], test);
					str[30]=r;
					radio.write_payload( &str[0], 32 );
					printf("sent the test case%d\n",test);
					//delay(3);
					
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

static uint32_t ntoi(uint8_t *buf) {
	return static_cast<uint32_t>(buf[0]) << 24
	| static_cast<uint32_t>(buf[1]) << 16
	| static_cast<uint32_t>(buf[2]) << 8
	| buf[3];
}

static uint16_t ntos(uint8_t *buf) {
	return static_cast<uint16_t>(buf[0]) << 8 | buf[1];
}