/*
 * FastSPI_LED2Receiver  RFPixel Control Receiver Sketch for handling the FAST_SPI2 Release candidate.
 * 
 *  Created on: Mar  2013
 *  Updated 6/2/2013
 *      Author: Greg Scull, komby@komby.com
 *      
 *      Users of this software agree to hold harmless the creators and contributors
 *      of this software. You, by using this software, are assuming all legal responsibility
 *      for the use of the software and any hardware it is used on.
 *
 * 
 * Some terminology to help you out
 * 
 * There are two uses of the term Channel in this code.  
 *
 * 1.  RF Channel - This is the channel that the RF Radio is using to communicate with the transmitter
 * 2.  DMX Channel - For a single universe of 512  this DMX Channel is one of those 512 channels.
 *    
 *    Note - Though I called it a DMX channel I want to point out that this code does not know anything about DMX.
 *            This code only knows about channels in relationship to the packet position which they occur.
 *
 *    What Protocol is this listening to?
 *
 *    The nRF24L01+ uses a variable sized payload for packets.  In the RFPixelControl library we are using the maximum
 *    of 32 Bytes.  These bytes are as follows
 *    -------------------------------------------------------------------------------------------------------------------
     | 1 | 2 | 3 | 4 | 5 | ...................................................................................| 31 | 32  |  
 *    -------------------------------------------------------------------------------------------------------------------
 *    Bytes 1-30 ( Index 0-29 ) Channel Data  Each byte represents 1 Channel of data,  One of the R , G, or B values.
 *    Byte  31 ( Index 30 )  - Sequence number,  This represents which order the packets are being sent
 *                             There will 18 Packets sent for an entire universe,  this index will contain 0-17.
 *    Byte 32   (Unused) 
 * 
 *      
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"



// REQUIRED VARIABLES
#define RECEIVER_UNIQUE_ID 33

 //What board are you using to connect your nRF24L01+?
 //Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
 #define NRF_TYPE  RF1_1_3


//#define RENARD_SERIAL_BAUD 57600
//#define PAD 0x7D
//#define SYNC 0x7E
//#define ESCAPE 0x7F
//#define COMMAND 0x80
#define REDPIN 10
#define GREENPIN 5
#define BLUEPIN 9


// Set OVER_THE_AIR_CONFIG_ENABLEG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for a short period after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

#define NUM_CHANNELS 3  
#define START_CHANNEL 1 //Where in the universe do we start
#define FINAL_CHANNEL 3 //DO Refactory out, addition would be easy....

#define DATA_BUFFER_LENGTH  3  //TODO Refactor so this is just the NUM_CHANNELS


#define LISTEN_CHANNEL 100 // the channel for the RF Radio




bool readytoupdate=false;

byte * buffer;

 
 
/**************END CONFIGURATION SECTION ***************************/

// Radio pipe addresses for the 2 nodes to communicate.
//these should not need to be modified
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//Uncomment for serial
#define DEBUG 0


//Arduino setup function.
void setup() {
  Serial.begin(57600);
   //  if (NUMBER_OF_DUMB_RGB_ADDON_CHANNELS){
         pinMode(REDPIN, OUTPUT);
         pinMode(GREENPIN, OUTPUT);
          pinMode(BLUEPIN, OUTPUT);
          
 buffer[0]=255;
    // }
    
    radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
   if(!OVER_THE_AIR_CONFIG_ENABLE)
   {
         int logicalControllerSequenceNum = 0;
         radio.AddLogicalController(logicalControllerSequenceNum, START_CHANNEL, NUM_CHANNELS,0);
        
   }
   printf_begin();
	
   	delay(2000);

     radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
      radio.printDetails(); 
      //initalize data buffer
      buffer= radio.GetControllerDataBase(0);

        delay (200);

  
        dumbRGBShow(255, 0, 0, 25);
         (2000);
        dumbRGBShow(0, 255, 0, 25);
        delay (200);
        dumbRGBShow(0, 0, 255, 25);
        delay (200);
   dumbRGBShow(0, 0, 0, 25);

}

//RF Listening to DMX packets loop
void loop(void){
   if( radio.Listen()){
     dumbRGBShow(buffer[0], buffer[1], buffer[2],1);
   }
    
  }
 

void dumbRGBShow( int r, int g, int b , int d) {
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
 delay(d);
}
