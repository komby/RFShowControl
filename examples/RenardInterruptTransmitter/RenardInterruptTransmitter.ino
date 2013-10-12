/*
 * Renard Transmitter
 * 
 *  Author: Greg Scull
 *  		7/2013
 *   This transmitter uses 
 */
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
/*************************** CONFIGURATION SECTION *************************************************/


//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,KOMBEE
#define NRF_TYPE			RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to transmit on?  
//Valid Values: 1-124
#define TRANSMIT_CHANNEL 100

//What is the baud rate of the incoming serial data?
#define RENARD_SERIAL_BAUD	57600


/***************************  END CONFIGURATION SECTION *************************************************/
#include <RFPixelControlConfig.h>
#define RF_NUM_PACKETS 18     // 18 * 30 total Channels can be broadcasted
#define RECEIVER_UNIQUE_ID 0
#define PIXEL_TYPE			NONE
#define PAD 0x7D
#define SYNC 0x7E
#define ESCAPE 0x7F
#define COMMAND 0x80

volatile unsigned int sub1=0;
volatile unsigned int sub2=0;
volatile unsigned char update;
volatile bool packetready=false;

//Initialize the RF packet buffer
byte str[RF_NUM_PACKETS][32];
int i=0;
int j=0;




void setup()
{
	radio.Initialize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID );
	UCSR0C |= (1<<USBS0);
	Serial.begin(RENARD_SERIAL_BAUD);

}


void loop()
{
    if (packetready )
       //If there is a packet ready to write to the radio...Write it!
       {
         packetready=false;
        radio.write_payload( &str[sub1-1], 32 );
        
        }
}

/**************************************************************************/
/*!
 This is the interrupt service handler for the serial data
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
     break;
  case COMMAND:  
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
  }
}
