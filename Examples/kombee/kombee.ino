/*
 * kombee.ino
 *
 *  
 *  Created on: Mar  2013
 *  Author: Greg Scull, komby@komby.com
 *     
 *  This sketch handles RF to Renard translation.  
 *  It can be used to connect an arduino, with a nRF24L01 transceiver
 *  to a Renard control board.
 *  
 *  This should work with arduino uno and compatible.
 *  If you like this code come check out  http://www.komby.com
 *  
 * Licensed under the Creative Commons Share Alike 3.0 License.
 *    
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"


//to use OTA configuration
#define  OVER_THE_AIR_CONFIG_ENABLE 0
#define RECEIVER_UNIQUE_ID 33  //make this different for each receiver.

#define RENARD_SERIAL_BAUD 57600
#define PAD 0x7D
#define SYNC 0x7E
#define ESCAPE 0x7F
#define COMMAND 0x80

#define NUM_CHANNELS 64  //Ren64 so i need 64 channels
#define START_CHANNEL 211 //Where in the universe do we start
#define FINAL_CHANNEL 275 //TODO Refactory out, addition would be easy....

#define DATA_BUFFER_LENGTH  128  //TODO Refactor so this is just the NUM_CHANNELS


#define LISTEN_CHANNEL 100 // the channel for the RF Radio

#define CHANNELS_PER_PACKET 30    //how many bytes of data are in each packet
#define PACKET_SEQ_IDX 30         //each packet has a sequence number,  what index 0-31 is it?
#define BYTES_PER_PACKET 30       //TODO refactor....
      

byte gotstr[32];
uint16_t counter=0;
uint16_t channelCounter=0;
int pkt_begin=0;
int  pkt_max=0;

bool readytoupdate=false;

uint8_t * renardData;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
// Orignial RF pins - 9, 10  (maniacbug)
RFPixelControl radio(8,7);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//Arduino setup function.
void setup() {
  Serial.begin(RENARD_SERIAL_BAUD);
  printf_begin();
  radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
   if(!OVER_THE_AIR_CONFIG_ENABLE)
   {
         int logicalControllerSequenceNum = 0;
         radio.AddLogicalController(logicalControllerSequenceNum, START_CHANNEL, NUM_CHANNELS,RENARD_SERIAL_BAUD);
          //renardData = new uint8_t[NUM_CHANNELS+2];
   }
 
 

  
 delay(200);
  radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,RECEIVER_UNIQUE_ID);
      renardData = radio.GetControllerDataBase(0);
  delay(200);

  radio.printDetails();
  
  //do some fancy stuff for the girls.....
  initStartupSequence();

}

//RF Listening to DMX packets loop
void loop(void){
   if (radio.Listen() )
    {
           sendRendardData();
    }
 }




//famcy startup routine for the ladies :)
void initStartupSequence(void) {
  
//First shut off all   
  Serial.write(SYNC);
   Serial.write(COMMAND);
   for (int i=0; i < NUM_CHANNELS; i++){
     renardData[i]=0x00;
   } 
 Serial.write(renardData, NUM_CHANNELS);
 delay(25);
 
//now iterate through all channels and turn on one at a time
 for (int j=0,  k=0;j<NUM_CHANNELS;j++){
      k = j; 
      renardData[k++]=0xFF;
       Serial.write(SYNC);
      Serial.write(COMMAND);
      Serial.write(renardData, NUM_CHANNELS);
  delay(25);  
 }  
    
  for (int j=NUM_CHANNELS-1,  k=0;j>=0 ;j--){
      k = j; 
      renardData[k--]=0x00;
      Serial.write(SYNC);
      Serial.write(COMMAND);
      Serial.write(renardData, NUM_CHANNELS);
  delay(25);  
  } 
}


/** 
*   sendRenardData -  outputs the renard data with escaping for control characters.
*/
void sendRendardData() {
      Serial.write(SYNC);
      Serial.write(COMMAND);
  for (int i = 0; i <= NUM_CHANNELS; i++)
  {
      switch(renardData[i])
      {
            case PAD:
             Serial.write( ESCAPE );
             Serial.write(0x2F );
           break;
            case SYNC:
               Serial.write(  ESCAPE );
               Serial.write(  0x30 );
            break;
            case ESCAPE:
               Serial.write(  ESCAPE );
               Serial.write(  0x31 );
            break;
            default :
               Serial.write( renardData[i] );  
          }
  }
}
