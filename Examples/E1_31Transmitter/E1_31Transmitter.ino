/*
 * RFDmxController.h
 *
 *  Created on: Mar 7, 2013
 *      Author: Greg
 */

#ifndef E131TRANSMITTER_H_
#define E131TRANSMITTER_H_

// Present a "Will be back soon web page", as stand-in webserver.
// 2011-01-30 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
//#include <tcpip.h>
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tcpip.h>

//#include "printf.h"
//#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)
//static BufferFiller bfill;  // used as cursor while filling the buffer
//#if STATIC
//// ethernet interface ip address
//static byte myip[] = { 192,168,1,76 };
//// gateway ip address
//static byte gwip[] = { 192,168,1,254};
//#endif
#define DMX_NUM_CHANNELS 512  // This must be divisible by 3!  This defines the number of DMX channels we are going to send.
#define RF_NUM_PACKETS 18     // This is DMX_NUM_CHANNELS divided by 30 rounded up.
#define DMX_CHANNEL_DATA_START  168//DMX Packet Position 0xA8
#define RF_INTERPACKET_DELAY         1150
volatile unsigned char dmx_state;
// this is the start address for the dmx frame
volatile unsigned int dmx_start_addr = 1;
// this is the current address of the dmx frame
volatile unsigned int dmx_addr;
// this is used to keep track of the channels
volatile unsigned int chan_cnt;
// array subscripts 1&2
//volatile unsigned int sub1=0;
//volatile unsigned int sub2=0;
// this holds the dmx data
//unsigned char dmx_data[DMX_NUM_CHANNELS];
// tell us when to update
//Initialize the RF packet buffer
byte str[32];

//#endif
static union {
  uint16_t PortNr;
  uint8_t PortByte[2];
}
myport;

uint16_t PortNr = 5568;

RFPixelControl radio(9,10);


// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[690]; // tcp/ip send and receive buffe

void setup(){
	//Serial.begin(57600);
	//Serial.println("\n[rf dmx REPEATER]");
       // printf_begin();



  
  
	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0);
	//	Serial.println( "Failed to access Ethernet controller");
//#if STATIC
//	ether.staticSetup(myip, gwip);
//#else
	while(!ether.dhcpSetup());
		//Serial.println("DHCP failed");
//#endif

	ether.printIp("IP:  ", ether.myip);
	ether.printIp("GW:  ", ether.gwip);
	ether.printIp("DNS: ", ether.dnsip);

	myport.PortByte[0] = 0x15;
	myport.PortByte[1] = 0xC0;

radio.Initalize( radio.TRANSMITTER, pipes, 100 );
	delayMicroseconds(150);

  radio.printDetails();
}
int maxprint=0;
int len=0;
void loop () {
	// get the next new packet:
        byte status = 0;
        len=0;
      len=ether.packetReceive();

           word pos = ether.packetLoop(len);
   
        
	//check to make sure that the packet received is a UDP && e1.31 packet.
	if (  len == 680 && ether.buffer[IP_PROTO_P] == IP_PROTO_UDP_V && ether.buffer[UDP_DST_PORT_H_P] == myport.PortByte[0] && ether.buffer[UDP_DST_PORT_L_P] == myport.PortByte[1] ) {

		//now we know we have a 31.31 packet what to do with it now
                      byte channel;
                      int channelPos=0;
                      int i = 0;
                      int pktCnt=0;
                      // unsigned long time = millis();
                        for (;channelPos< DMX_NUM_CHANNELS;){           
                          for (i=0;i<30 && channelPos<512; i++, channelPos++ ){
                             str[i]=ether.buffer[DMX_CHANNEL_DATA_START+channelPos];
                             
                          }
                          str[i]=pktCnt++;//setpacketCounter
                          radio.write_payload( &str[0], 32 );
                           status = radio.get_status();
                          delayMicroseconds(RF_INTERPACKET_DELAY);
                          while (status & 0x01) {
                            status = radio.get_status();
                          //  printf("Waiting on Transmit\n\r");
      }
                        }
                    
        
      
      }

}








#endif /* RFDMXCONTROLLER_H_ */

