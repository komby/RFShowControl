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

//How many pixels do you want to transmit data for
#define NUM_CHANNELS 512
#define RF_DELAY      2000

//use channel 100
#define TRANSMIT_CHANNEL 100

//Setup  a RF pixel control
//RF1 v.01 board uses Radio 9,10
//ARDUINO ETHERENET NEEDS PIN 10 Relocate CSN to pin8
RFPixelControl radio(9,8);

//kombyone due transmitter board radio settings.
//RFPixelControl radio(33,10);

/***************************  CONFIGURATION SECTION *************************************************/

//change the mac (0xEF, 0xE0, 0xE1, ... 0xE9)
byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change this IP to one on your network
IPAddress ip(192, 168, 2, 76);



// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
#define RF_NUM_PACKETS 18  


//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START  126//DMX Packet Position 0xA8


char packetBuffer[700]; //buffer to hold incoming packet,
byte str[32];
EthernetUDP Udp;

void setup(){
	
uint16_t PortNr = 5568;
	Ethernet.begin(mac,ip);
	Udp.begin(PortNr);

	Serial.begin(57600);
	Serial.println("\n[E1.31 Arduino Ethernet Transmitter ]\n");
	printf_begin();
	radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL );
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
	if(packetSize == 638)//todo refactor to check the packet better
	{  
                
		Udp.read(packetBuffer,700);
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
