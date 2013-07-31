/*
* RFDMXTransmitter
*
*  Created on: Mar  2013
*      Author: Greg Scull, komby@komby.com
*
*      This code is a derivative of the original work done by
*      Joe Johnson RFColor_24 Receiver Program
*
*      The Code which Joe wrote inspired this software, the related hardware and
*      it was also used as a starting point for this class.
*
*      As with the RFColor_24 The commercial Use of this software is Prohibited.
*      Use this software and (especially) the hardware at your own risk.

*      Users of this software agree to hold harmless the creators and contributors
*      of this software.  By using this software you agree that you are doing so at your own risk, you
*      could kill yourself or someone else by using this software and/or modifying the
*      factory controller.  You, by using this software, are assuming all legal responsibility
*      for the use of the software and its implicit hardware.
*
*      The Commercial Use of this Software is Prohibited.
*
*
*       To Use this transmitter code you need to modify the HardwareSerial library from your main arduino install location
*       for instructions on how to do so visit: http://learn.komby.com/wiki/20/hardware-serial-modification
*
*/
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
/***************************  BEGIN CONFIGURATION SECTION *************************************************/
#define DMX_NUM_CHANNELS 512  // This must be divisible by 3!  This defines the number of DMX channels we are going to listen to.
#define RF_NUM_PACKETS 18     // This is DMX_NUM_CHANNELS divided by 30 rounded up.

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE			RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to transmit on?  
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 100
/***************************  END CONFIGURATION SECTION *************************************************/

//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>
enum
{
	DMX_IDLE,
	DMX_BREAK,
	DMX_START,
	DMX_RUN
};

volatile unsigned char dmx_state;
// this is the start address for the dmx frame
volatile unsigned int dmx_start_addr = 1;
// this is the current address of the dmx frame
volatile unsigned int dmx_addr;
// this is used to keep track of the channels
volatile unsigned int chan_cnt;
// array subscripts 1&2
volatile unsigned int sub1=0;
volatile unsigned int sub2=0;
// this holds the dmx data
unsigned char dmx_data[DMX_NUM_CHANNELS];
// tell us when to update
volatile unsigned char update;
volatile bool packetready=false;
//Initialize the RF packet buffer
byte str[RF_NUM_PACKETS][32];
int i=0;
int j=0;



int fred;


void setup()
{
	
	radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL,DATA_RATE ,0);

	delayMicroseconds(150);
	update = 0;
	// set default DMX state
	dmx_state = DMX_IDLE;
	// initialize UART for DMX
	// this will be 250 kbps, 8 bits, no parity, 2 stop bits
	UCSR0C |= (1<<USBS0);
	Serial.begin(250000);  //Start serial communications at 250,000 Baud.  NOTE: For this to work for DMX the HardwareSerial.cpp file must be modified to remove the interrupt code.
	fred=50;
}


void loop()
{
	if (packetready )
	//If there is a packet ready to write to the radio...Write it!
	{
		radio.write_payload( &str[sub1-1], 32 );
		packetready=false;
	}
}

/**************************************************************************/
/*!
This is the interrupt service handler for the DMX
*/
/**************************************************************************/
ISR(USART_RX_vect)
{
	unsigned char status = UCSR0A;
	unsigned char data = UDR0;

	switch (dmx_state)
	{
		case DMX_IDLE:
		if (status & (1<<FE0))
		{
			dmx_addr = 0;
			dmx_state = DMX_BREAK;  //Once we get a break, that means we are about to read a new DMX packet.
		}
		break;

		case DMX_BREAK:
		if (data == 0)
		{
			dmx_state = DMX_START;
		}
		else
		{
			dmx_state = DMX_IDLE;
		}
		break;

		case DMX_START:
		dmx_addr++;
		if (dmx_addr == dmx_start_addr)
		{
			chan_cnt = 0;  //Reset Channel Count
			sub1=0;  //Reset array subscripts to 0
			sub2=0;
			str[sub1][sub2]=data;
			chan_cnt++;
			sub2++;;
			dmx_state = DMX_RUN;
		}
		break;

		case DMX_RUN:
		str[sub1][sub2]=data;
		chan_cnt++;  //increment channel
		sub2++;  //increment packet byte counter
		if ((sub2==30) || (chan_cnt>=DMX_NUM_CHANNELS))  //if byte counter =30, reset
		{
			str[sub1][30]=sub1;   //set packet number in byte 30
			sub2=0;               //reset packet byte counter
			sub1++;               //increment packet number
			packetready=true;     //flag that packet is ready to transmit
		}
		if (chan_cnt >= DMX_NUM_CHANNELS)  //Once we have gotten all the channels we want, quit listening....
		{
			dmx_state = DMX_IDLE;
			if (sub2>0)  //Check to see if we got here with a partial final packet we didn't yet send....  If we did, send it now.
			{
				sub2=0;
				str[sub1][30]=sub1;
				sub1++;
				packetready=true;
			}
		}
		break;

		default:
		dmx_state = DMX_IDLE;
		break;
	}
}
