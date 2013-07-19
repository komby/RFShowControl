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
#include "FastSPI_LED2.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

 
 /**************CONFIGURATION SECTION ***************************/
//Step 1.
//Radio Configuration - 
//What Channel do you want this nRF24L01+ to listen on?
#define LISTEN_CHANNEL 100 

//Step 2.
//What Pins is your nRF24L01+ CE and CSN connected to?  
//Usage:   RFPixelControl radio(CE, CSN);

//RF1 v0.2 & RF1_12V 0.1 PCB ises( PB0, Arduino Pin 8  for CE ) (PD7 Arduino pin 7 for CSN)
RFPixelControl radio(8,7);
//RFPixelControl radio(9,10); //RF1 Version 0.1 (maniacbug wiring)



//Step 3.
//How Many Pixels are in your string?  This is the number of pixels not the number of LEDs or channels 
#define NUM_LEDS 50
#define NUM_LEDS_PER_PIXEL 3

struct CRGB leds[NUM_LEDS];
//Step 4.
//How Bright should our LEDs start at
#define LED_BRIGHTNESS 64 //25%

//Step 5.
//Which channels are you planning for using on this controller?  Most transmitters are broadcasting 512 Channels
//Use this range (starting at 0)

//For example if this is the first string of lights  listening to nRF24L01+ RFChannel 100 starting at DMX channel 0
//It will use channels 0-149 
#define DMX_START_CHANNEL 0

//Second controller listening to channels 150-349
//#define DMX_START_CHANNEL 150

//  One More Step - This one is in the Setup Method 
//  Scroll down to Step 6.

 /**************END CONFIGURATION SECTION ***************************/

// Radio pipe addresses for the 2 nodes to communicate.
//these should not need to be modified
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//Uncomment for serial
#define DEBUG 0

//create the pixel control impl with the  driver
 //Fast_SPI2_RFPixelControl strip =  Fast_SPI2_RFPixelControl();

byte gotstr[32];
uint16_t counter=0;
uint8_t led_counter=0;
uint16_t dmx_counter=0;
int pkt_begin=0;
int  pkt_max=0;
int  z=0;
bool readytoupdate=false;




//Arduino setup function.
void setup() {
  Serial.begin(57600);
  
   printf_begin();
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);

      radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL);
      radio.printDetails(); 
   	LEDS.setBrightness(LED_BRIGHTNESS);
//Step 6.  
//Choose your light controller  Comment/Uncomment as needed

        
	// Put ws2801 strip on the hardware SPI pins with a BGR ordering of rgb and limited to a 1Mhz data rate
	LEDS.addLeds<WS2801, 2, 4, RGB, DATA_RATE_MHZ(1)>(leds, NUM_LEDS);

   	// LEDS.addLeds<LPD8806, 2, 4>(leds, NUM_LEDS);
   	// LEDS.addLeds<WS2811, 13, BRG>(leds, NUM_LEDS);
   	// LEDS.addLeds<LPD8806, BGR>(leds, NUM_LEDS);
   
        
        //Unused Hardware SPI 
 	// LEDS.addLeds<WS2811, 13>(leds, NUM_LEDS);
   	// LEDS.addLeds<TM1809, 13>(leds, NUM_LEDS);
   	// LEDS.addLeds<UCS1903, 13>(leds, NUM_LEDS);
   	// LEDS.addLeds<TM1803, 13>(leds, NUM_LEDS);

   	//LEDS.addLeds<LPD8806>(leds, NUM_LEDS)->clearLeds(300);
	// LEDS.addLeds<WS2801>(leds, NUM_LEDS);
   	// LEDS.addLeds<SM16716>(leds, NUM_LEDS);
   
   	// LEDS.addLeds<WS2811, 11>(leds, NUM_LEDS);
//All done.  You dont need to modify anything else.
        
       //Initalize the data for LEDs
        memset(leds, 0,  NUM_LEDS * sizeof(struct CRGB));
      
        delay (200);
}

void loop(void){
   int updateChannelCount = 0;
// See if there is any data in the RF buffer
 if ( radio.available() ){

    for (bool done = false;!done;){
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &gotstr, 32 );
      
      //Update the led_counter value from the packet address in position 30
      pkt_begin=gotstr[30]*30;//0=0.1=30.2=60
      dmx_counter=pkt_begin;
      pkt_max=pkt_begin+29;
      z=0;
   
      int dmxEnd = DMX_START_CHANNEL + (NUM_LEDS * NUM_LEDS_PER_PIXEL);//150 for a 50 pixel string
      //printf("%d dmx end\n", dmxEnd);
      //Make sure the numbers make sense
        if (pkt_max>=DMX_START_CHANNEL && pkt_begin< dmxEnd) 
      {
     
        /* This loop is checking to make sure that the packet being looked at
         				 * is a channel which is in the subset of channels from the DMX Universe
         				 * that this pixel string is in.   If the packet is outside that range
         				 * it is skipped.
         				 */
        for (;(dmx_counter<DMX_START_CHANNEL && z<30)||dmx_counter>=dmxEnd;z++, dmx_counter++){}
          led_counter=(dmx_counter-DMX_START_CHANNEL)/3; 
          for(;   DMX_START_CHANNEL <= dmx_counter && dmx_counter < dmxEnd && z<30;){
               leds[led_counter]= CRGB(gotstr[z], gotstr[z+1], gotstr[z+2]);
               dmx_counter+=3;
               led_counter++;
               if ((dmx_counter>=dmxEnd) || (dmx_counter>=508)){
                 readytoupdate=true;
          }
            //Move the index counter past the 3 LEDs of the current pixel.
            z=z+3;
        }
        
        
        //If we have received a full set of data, Update all the LED's that have changed value since the last time
        if(readytoupdate)
           LEDS.show();
           readytoupdate=false;
      }
    }
  }
}
