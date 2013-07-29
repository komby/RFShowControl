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
 */
// REQUIRED VARIABLES
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE  RF1_1_3

//What Kind of pixels? Valid Values: LPD_6803
#define PIXEL_TYPE			LPD_6803
#define PIXEL_DATA_PIN 2
#define PIXEL_CLOCK_PIN 4

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

// Set OVER_THE_AIR_CONFIG_ENABLEG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for a short period after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0


// If you are not using Over the air configuration you will need to specify the following options
 #define HARDCODED_START_CHANNEL 0
 #define HARDCODED_NUM_PIXELS 50  //This defines the # of LED Channels.
 #define HARDCODED_NUM_CHANNELS 150 //Number of DMX channels to read...usually dmx_led_channels/3
 #define LISTEN_CHANNEL 100 // the channel for the RF Radio





//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>



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

  Serial.write("Initializing Radio\n");

 radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
  radio.printDetails(); 
  Serial.write("Init and Paint LEDS for startup \n");
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



