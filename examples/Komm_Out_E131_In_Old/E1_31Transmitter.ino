/*
 * ENC28j60 based controller for E1.31 and RFPixel control
 *
 *  Created on: Mar 7, 2013
 *      Author: Greg
 */

// 2011-01-30 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
//#include <tcpip.h>
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tcpip.h>
#include <EEPROM.h>
/***************************  CONFIGURATION SECTION *************************************************/
//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE MINIMALIST_SHIELD

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS
//How many pixels do you want to transmit data for
//#define NUM_CHANNELS 512
#define RF_DELAY      2000

//use channel 100
#define TRANSMIT_CHANNEL 100


/***************************  CONFIGURATION SECTION *************************************************/
#include <RFPixelControlConfig.h>
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

//Initialize the RF packet buffer
byte str[32];

//#endif
static union {
  uint16_t PortNr;
  uint8_t PortByte[2];
}
myport;

uint16_t PortNr = 5568;



// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[690]; // tcp/ip send and receive buffe

void setup(){

	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0);

	while(!ether.dhcpSetup());

	ether.printIp("IP:  ", ether.myip);
	ether.printIp("GW:  ", ether.gwip);
	ether.printIp("DNS: ", ether.dnsip);

	myport.PortByte[0] = 0x15;
	myport.PortByte[1] = 0xC0;


radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL,DATA_RATE ,0);

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