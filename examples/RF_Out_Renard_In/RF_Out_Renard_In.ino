/*
 * Renard Transmitter
 *
 * TODO
 *
 *  Author: Greg Scull
 *  		7/2013
 *   This transmitter uses 
 */
//TODO HEADER CHECK
#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
// Valid Values: RF1, MINIMALIST_SHEILD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE					RF1
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// TRANSMIT_CHANNEL Description: http://learn.komby.com/Configuration#Transmit_Channel
// Valid Values: 1-124
#define TRANSMIT_CHANNEL				100

// DATA_RATE Description: http://learn.komby.com/wiki/Configuration#Data_Rate
// Valid Values: RF24_250KBPS, RF24_1MBPS 
#define DATA_RATE					RF24_250KBPS

// RENARD_BAUD_RATE Description: http://learn.komby.com/Configuration#Renard_Baud_Rate
// Valid Values: 19200, 38400, 57600, 115200, 230400, 460800
#define RENARD_BAUD_RATE			57600
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
//#define DEBUG						1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define RF_WRAPPER 1
//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

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
	Serial.begin(RENARD_BAUD_RATE);

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
/*
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
*/
