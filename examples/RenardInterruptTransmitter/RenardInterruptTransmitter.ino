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
 *       TO Use this transmitter code you need to modify the HardwareSerial library from your main arduino install location
 *       for instructions on how to do so you can follow step "7.1.3. Hardwareserial.cpp"  from
 *       the PDF writeup for the RFColor2_4 - http://doityourselfchristmas.com/forums/attachment.php?attachmentid=18291&d=1361578971
 *
 */


#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define DMX_NUM_CHANNELS 512  // This must be divisible by 3!  This defines the number of DMX channels we are going to listen to.
#define RF_NUM_PACKETS 18     // This is DMX_NUM_CHANNELS divided by 30 rounded up.

//#define TRANSMIT_CHANNEL 100     // This is the channel we are broadcasting on.
/***************************  CONFIGURATION SECTION *************************************************/
//use channel 100
#define TRANSMIT_CHANNEL 100

//Setup  a RF pixel control 
//RF1 v.01 board uses Radio 9,10
RFPixelControl radio(8,7);
//kombyone due transmitter board radio settings.
//RFPixelControl radio(33,10);


#define RENARD_SERIAL_BAUD	57600
//How many pixels do you want to transmit data for
#define NUM_CHANNELS		464
/***************************  CONFIGURATION SECTION *************************************************/
enum
{
  IDLE,
  BREAK,
  START,
  SYNC, 
  RUN
};

#define PAD 0x7D
#define SYNC 0x7E
#define ESCAPE 0x7F
#define COMMAND 0x80

volatile unsigned char state;
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


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
//RFPixelControl radio(9,10);

//RF1 controllers uncomment this line
//RFPixelControl radio(8,7);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
int fred;


void setup()
{
  radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, RF24_1MBPS, 0 );
	
//	radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL);
	delayMicroseconds(150);
	update = 0;
	// set default DMX state
	//dmx_state = DMX_IDLE;
	// initialize UART for DMX
	// this will be 250 kbps, 8 bits, no parity, 2 stop bits
	UCSR0C |= (1<<USBS0);
Serial.begin(57600);
	//Serial.begin(250000);  //Start serial communications at 250,000 Baud.  NOTE: For this to work for DMX the HardwareSerial.cpp file must be modified to remove the interrupt code.
	fred=50;
}


void loop()
{
    if (packetready )
       //If there is a packet ready to write to the radio...Write it!
       {
         packetready=false;
        radio.write_payload( &str[sub1-1], 32 );
        
        //delay(2);
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

  switch (data)
  {
  
  case SYNC:
  

      if (sub2>0 && sub2 < 30)  //Check to see if we got here with a partial final packet we didn't yet send....  If we did, send it now.
         {
           
            str[sub1++][30]=sub1;
            
            packetready=true;
         }
   
   //reset the channel counters
   
//      str[sub1][sub2]=data;
//      chan_cnt++;
//      sub2++;;
     break;
  case COMMAND:  
       dmx_addr = 0;
       chan_cnt = 0;  //Reset Channel Count
      
      sub2=0;
            sub1=0;  //Reset array subscripts to 0
            sub2=0;
        break;
    
  case PAD:
    delayMicroseconds(1);
    break;

  case ESCAPE:
      delayMicroseconds(1);
        break;

default:
  str[sub1][sub2]=data;
  sub2++;  //increment packet byte counter
  if ((sub2==30) )  //if byte counter =30, reset
     {
      str[sub1][30]=sub1;   //set packet number in byte 30
      sub2=0;               //reset packet byte counter
      sub1++;               //increment packet number
      packetready=true;     //flag that packet is ready to transmit
     }
    break;

//  default:
//    dmx_state = IDLE;
//    break;
  }
}
