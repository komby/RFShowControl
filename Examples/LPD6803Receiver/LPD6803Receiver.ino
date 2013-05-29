/*
 * LPD6803RFReceiver.cpp
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
 *      The Commercial Use of this Software is Prohibited. 
*      To use this software you will need the Adafruit LPD6803 Library
*      https://github.com/adafruit/LPD6803-RGB-Pixels
*       You will also need to download the TimerOne library
*        https://code.google.com/p/arduino-timerone/downloads/detail?name=TimerOne-v9.zip&can=2&q=
 */


#include <Arduino.h>
#include <TimerOne.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include "LPD6803PixelControl.h"
#ifndef ADAFRUITILIB6803
#define ADAFRUITILIB6803
#include <LPD6803.h>
#endif
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
/**
 * The LPD6803 receiver is only designed to control 1 string of lights at a time.
 * 
 * NOTE:  I do not have any LPD6803 pixels to test this with I simply made sure it looked like the code would work. 
 * Use at your own risk.
 * As the receiver is listening to a Transmitter capable of transmitting 512 channels
 * We need to make sure that we are listening to the right channels in the universe
 *
 * DMX_START_CHANNEL -   This is the first channel that will be used for this string
 * 					      if this is the first string in the universe it should be set to (0 or 1)//TODO Determine 0 or 1
 * DMX_NUM_CHANNELS - 	 This is the total number of channels needed for this string. For RGB (3) x number of pixels = #
 * DMX_LED_CHANNELS 20 - This defines the # of LED Channels.  This could be the same as lightCount, but if grouping is used, this will be less.
 */
#define DMX_START_CHANNEL 0

#define DMX_LED_CHANNELS 50  //This defines the # of LED Channels. 

#define DMX_NUM_CHANNELS 150 //Number of DMX channels to read...usually dmx_led_channels/3

#define LISTEN_CHANNEL 100 // the channel for the RF Radio


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RFPixelControl radio(9,10);

//create the pixel control impl with the LPD6803 driver
LPD6803PixelControl  strip =  LPD6803PixelControl();

#define DEBUG 0

byte gotstr[32];
uint16_t counter=0;
uint8_t led_counter=0;
uint16_t dmx_counter=0;
int pkt_begin=0;
int  pkt_max=0;
int  z=0;
bool readytoupdate=false;


// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


//Arduino setup function.
void setup() {
  Serial.begin(57600);
  printf_begin();
  strip.SetPixelCount(DMX_LED_CHANNELS );
  strip.Start();


  Serial.write("Initializing Radio\n");

  radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL);
  radio.printDetails(); 
  Serial.write("Init and Paint LEDS for startup \n");
   strip.ColorWipe(strip.Color(255, 255, 0), 25);
   strip.Paint();
   strip.ColorWipe(strip.Color(255, 0,255), 25);
   strip.Paint();
  delay (2);
  
  for(int i=0;i<strip.GetPixelCount(); i++)
    strip.SetPixelColor(i, strip.Color(0,0,0));
    strip.Paint();
    delay(2);
}


int maxprint=0;
//RF Listening to DMX packets loop
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
               
            strip.SetPixelColor(led_counter, strip.Color(gotstr[z], gotstr[z+1], gotstr[z+2]));
        
            dmx_counter+=3;
            led_counter++;
            if ((dmx_counter>=dmxEnd) || (dmx_counter>=508)){
              readytoupdate=true;
            }
  
            //Move the index counter past the 3 LEDs of the current pixel.
            z=z+3;
        }
        
        
        //If we have received a full set of data, Update all the LED's that have changed value since the last time
        //
        if(readytoupdate)
          strip.Paint();
         readytoupdate=false;
        
      }
    }
  }
}



