/*
 * FastSPI_LED2Receiver
 *
 *  Created on: Mar  2013
 *      Author: Greg Scull, komby@komby.com
 *      
 *      Users of this software agree to hold harmless the creators and contributors
 *      of this software. You, by using this software, are assuming all legal responsibility
 *      for the use of the software and any hardware it is used on.
 *
 *      This code is using the adafruit library without using the HardwareSPI  of the adafruit lib.
 *      The pins on the atmega used are pins 4 & 5 for clock and data.
 * 
 *      
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include "Fast_SPI2_RFPixelControl.h"
#include <FastSPI_LED2.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/**
 * This receiver is only designed to control 1 string of lights at a time.
 *
 * As the receiver is listening to a Transmitter capable of transmitting 512 channels
 * We need to make sure that we are listening to the right channels in the universe
 *
 * DMX_START_CHANNEL -   This is the first channel that will be used for this string
 * 					      if this is the first string in the universe it should be set to (0 or 1)//TODO Determine 0 or 1
 * DMX_NUM_CHANNELS - 	 This is the total number of channels needed for this string. For RGB (3) x number of pixels = #
 * DMX_LED_CHANNELS 20 - This defines the # of LED Channels.  This could be the same as lightCount, but if grouping is used, this will be less.
 */
 
 
 /**************CONFIGURATION SECTION ***************************/
#define DMX_START_CHANNEL 0

#define DMX_LED_CHANNELS 50  //This defines the # pixels in your string

#define DMX_NUM_CHANNELS 150 //Number of DMX channels to read...usually dmx_led_channels/3

#define LISTEN_CHANNEL 100 // the channel for the RF Radio

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10  
// RF1 v0.1 hardware uses pin 9,10 
RFPixelControl radio(9,10);


//In the next grouping you need to choose your pixel tipe and uncomment it.
//by default it is using the WS2801 you will need to comment that controller out if you are using a different one.

//CREATE A fast spi2 CONTROLLER  to wrap in RF functionality.
//LIKELY HARDWARE SPI NO SUPPORT
//LPD8806Controller<11, 13, 10> *  LED  = new LPD8806Controller<11, 13, 10>() ;
// SM16716Controller<11, 13, 10>*  LED = new SM16716Controller<11, 13, 10>();
//LPD8806Controller<11, 13, 14>* LED = new LPD8806Controller<11, 13, 14>();
// LPD8806Controller<12, 13, 10, 0>* LED;


/* NOT TESTED BUT SHOULD WORK */
// Same Port, non-hardware SPI - 1.2ms for an 86 led frame, 1.12ms with large switch 
// r2 - .939ms without large switch  .823ms with large switch
// r3 - .824ms removing 0 balancing nop, .823 with large switch removing balancing
// LPD8806Controller<12, 13, 10>*  LED = new LPD8806Controller<12, 13, 10>();

/* NOT TESTED BUT SHOULD WORK */
// Different Port, non-hardware SPI - 1.47ms for an 86 led frame
// Different PortR2, non-hardware SPI - 1.07ms for an 86 led frame
// LPD8806Controller<7, 13, 10>* LED= new LPD8806Controller<7, 13, 10>();
// LPD8806Controller<8, 1, 10>* LED = new LPD8806Controller<8, 1, 10>() ;
// LPD8806Controller<11, 14, 10>* LED = new  LPD8806Controller<11, 14, 10>();

// Same Port, non-hardware SPI - 1.2ms for an 86 led frame, 1.12ms with large switch 
// WS2801Controller<11, 13, 10, 0> LED;
WS2801Controller<2, 3, 8, 16> *   LED = new WS2801Controller<2, 3, 8, 16>() ;

// WS2801Controller<7, 13, 10> * LED;

// TM1809Controller800Mhz<6> * LED = new TM1809Controller800Mhz<6>();
// UCS1903Controller400Mhz<7> * LED= new UCS1903Controller400Mhz<7>();
// WS2811Controller800Mhz<12> * LED= new WS2811Controller800Mhz<12>();
// WS2811Controller800Mhz<5> * LED= new WS2811Controller800Mhz<5>();
// TM1803Controller400Mhz<5> *  LED= new TM1803Controller400Mhz<5>();

// Radio pipe addresses for the 2 nodes to communicate.
//these should not need to be modified
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


 /**************END CONFIGURATION SECTION ***************************/

//Uncomment for serial
#define DEBUG 0

//create the pixel control impl with the  driver
 Fast_SPI2_RFPixelControl strip =  Fast_SPI2_RFPixelControl();

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
 // printf_begin();

   radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL);
  radio.printDetails(); 

  delay (200);
  strip.SetPixelCount(DMX_LED_CHANNELS );
  strip.InitFastSPI( (CLEDController *)  LED);
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
    
      /* This is the current packet format,  it may be revised
       * pending the outcome of the packet DMX format on DICY
       * z is the packet byte counter
       * dmx data is in packet bytes 0 to 29.
       * Packet address is in byte 30.
       * Packet byte 31 is not used yet.
       */
      z=0;
      byte red = 0;
      byte green = 0;
      byte blue = 0;
      int dmxEnd = DMX_START_CHANNEL + DMX_NUM_CHANNELS;  //150 for a 50 pixel string
      
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
         for(;   DMX_START_CHANNEL <= dmx_counter && dmx_counter < dmxEnd && z<30;)     
         {
               red=gotstr[z];
               green = gotstr[z+1];
               blue = gotstr[z+2];
               strip.SetPixelColor(led_counter, strip.Color(red, green, blue));
        
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
          strip.Paint();
           readytoupdate=false;
           delay(2);     
      }
    }
  }
}


