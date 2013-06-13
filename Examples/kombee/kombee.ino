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

byte renardData[DATA_BUFFER_LENGTH];

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
  
 delay(200);
  radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL);
  delay(200);
 renardData[0]=   SYNC;
 renardData[1]= COMMAND;
 delay(200);
  //radio.printDetails();
  
  //do some fancy stuff for the girls.....
  initStartupSequence();

}

//because subtraction is annoying,  ill confuse myself with some additional variables...
int startChannel = START_CHANNEL-1;
int finalChannel = FINAL_CHANNEL - 1;

//RF Listening to DMX packets loop
void loop(void){
  // See if there is any data in the RF buffer
  if ( radio.available() ){
    for (bool done = false;!done;){
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &gotstr, 32 );
        //when process packet returns true we got the last channel we are listening to and its time to output....
       if (processPacket( renardData, gotstr))
         {
           //Fire up the serial
           sendRendardData();
         }
      }
    }
  }



  /**
  *  processPacket - handles copying the needed channels from the radio data into the Renard data.
  *        This method takes a different approach for getting the data.  Originally I was looping through
  *        all the channels and I got bored.  So I wrote this convoluded code hoping to make things faster by using memcpy
  *        I havent done performance timining so who knows?  It passed my tests :)
  *
  *  return - true if the last channel was found and its time for an update, otherwise false.
  */
boolean  processPacket(byte*  dest, byte* p)
{
//  Way too many variables here need to refactor...
  boolean retVal = false;
  int packetSequence = p[PACKET_SEQ_IDX];
  int packetStartChann = packetSequence * CHANNELS_PER_PACKET;
  int packetEndChannel = packetStartChann + CHANNELS_PER_PACKET;

  int calcStartChannel = -1;
  int calcStartDestIdx = -1;
  int calcStartSourceIdx = -1;


  int calcEndChannel = -1;
  int calcEndDestIdx = -1;
  int calcEndSourceIdx = -1;
 
  //first assume we will be copying the whole packet.
  int channelCopyStartIdx = 0;
  int channelCopyEndIdx = CHANNELS_PER_PACKET - 1;
  int numberOfValidChannelsInPacket = channelCopyEndIdx;

  //First we need to know what is the first valid channel in this packet
  if (startChannel >= packetStartChann)
  {
      //It could be in the packet is it?
      if (startChannel <= packetEndChannel)
      {
          //set calculated start to the start channel 
          calcStartChannel = startChannel;
          //set the idx of the dest array to the start channel
          calcStartDestIdx = 0 ;
          //offset the idx for source by the chanel count factor
          calcStartSourceIdx = startChannel - packetStartChann;
          //start is after the packet start and before or equal to the end channel.
         
      }
      else
      {
          //start channel was after the range of channels in this packet.  skip this packet.
          return retVal;
      }
  }
  else
  {
      calcStartChannel =  packetStartChann;
      calcStartDestIdx = packetStartChann - startChannel;
      calcStartSourceIdx = 0;
    
      //we started before this packet
      //end channel interrogation will handle if we need
      //anything from this packet.
  }
  //now interrogate the end channel
  //check if the end is in this packet
  if (packetEndChannel <= finalChannel)
  {
      //since final is greater than the end of the packet, check start
      //if start is set we need all channels from this packet.
      if (calcStartChannel >= 0)
      {
          calcEndChannel = packetEndChannel;
          calcEndDestIdx = packetEndChannel - (startChannel);
          calcEndSourceIdx = packetEndChannel - packetStartChann;
      }

  }
  else
  {
      //final channel is before the end of thi spacekt  (packetEndChannel >= finalChann ) 
      if (finalChannel >= packetStartChann)
      {
          //Because final is less than the packet end
          //and >= start we know that the final channel is in this packet
          calcEndChannel = FINAL_CHANNEL ;
          calcEndDestIdx = finalChannel - startChannel ;
          calcEndSourceIdx = finalChannel - packetStartChann;
          
          //because this is the last packet we care about,  return true
          retVal=true;
          
      }
      else
      {
          //Final happened before this packet//continue.
            return retVal;
      }
  }

  //now we have calculated positions
  if (calcStartChannel >= 0 && calcEndChannel >= 0)
  {
      //how many channels are we getting from this packet?
      int numChannelsInPacket = calcEndChannel - calcStartChannel;

      //Use memcpy to copy the bytes from the radio packet into the renard data array.
      memcpy(&dest[calcStartDestIdx], &p[calcStartSourceIdx], numChannelsInPacket);
     
  }
  return retVal;
}


//famcy startup routine for the ladies :)
void initStartupSequence(void) {

//First shut off all   
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
