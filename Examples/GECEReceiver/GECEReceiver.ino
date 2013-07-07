/*
 * GECERFReceiver.cpp
 *
 *  Created on: Mar  2013
 *      Author: Greg Scull, komby@komby.com
 *      
 *      Users of this software agree to hold harmless the creators and contributors
 *      of this software. You, by using this software, are assuming all legal responsibility
 *      for the use of the software and any hardware it is used on.
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include "GECEPixelControl.h"
#include <GEColorEffects.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
/**
 * The GECE receiver is only designed to control 1 string of lights at a time.
 *
 * As the receiver is listening to a Transmitter capable of transmitting 512 channels
 * We need to make sure that we are listening to the right channels in the universe
 *
 * DMX_START_CHANNEL -   This is the first channel that will be used for this string
 * 					      if this is the first string in the universe it should be set to(0 or 1)//TODO Determine 0 or 1
 * DMX_NUM_CHANNELS - 	 This is the total number of channels needed for this string. For RGB(3) x number of pixels = #
 * DMX_LED_CHANNELS 20 - This defines the # of LED Channels.  This could be the same as lightCount, but if grouping is used, this will be less.
 */
//Step 0.  Over The Air Configuration Enabled
//change the 1 to a 0 if you do not want
//to use OTA configuration
#define  OVER_THE_AIR_CONFIGURATION 0

#define RECEIVER_UNIQUE_ID 50  //make this different for each receiver.

#define DMX_START_CHANNEL 0

#define DMX_LED_CHANNELS 5	//This defines the # of LED Channels.

#define DMX_NUM_CHANNELS 15	//Number of DMX channels to read...usually dmx_led_channels*3

#define LISTEN_CHANNEL 100	// the channel for the RF Radio


#define  OVER_THE_AIR_CONFIGURATION 1



//Uncomment for serial
#define DEBUG 0

byte gotstr[32];
uint16_t counter = 0;
uint8_t led_counter = 0;
uint16_t dmx_counter = 0;
int pkt_begin = 0;
int pkt_max = 0;
int z = 0;
bool readytoupdate = false;

  GECEPixelControl strip = GECEPixelControl();
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
//RFPixelControl radio(9, 10);
//RF1 v0.2 & RF1_12V 0.1 PCB ises( PB0, Arduino Pin 8  for CE ) (PD7 Arduino pin 7 for CSN)
RFPixelControl radio(8,7);



// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};


//Arduino setup function.
void setup()
{
  Serial.begin(57600);
  printf_begin();
  //strip.SetPixelCount(DMX_LED_CHANNELS);

  Serial.write("Initializing Radio\n");
 if(!OVER_THE_AIR_CONFIGURATION)
 {
	 radio.AddLogicalController(RECEIVER_UNIQUE_ID, DMX_START_CHANNEL, DMX_NUM_CHANNELS, 0);
 }
  //radio.Initalize(radio.RECEIVER, pipes, LISTEN_CHANNEL);
  radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,RECEIVER_UNIQUE_ID);
  radio.printDetails();
	
  uint8_t logicalControllerNumber = 0; 

  strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber));

  for (int i = 0; i < strip.GetPixelCount(); i++)
    strip.SetPixelColor(i, strip.Color(0, 0, 0));
  strip.Paint();

  delay(200);
}



//RF Listening to DMX packets loop
void loop(void)
{
    
    //When Radio.Listen returns true its time to update the LEDs for all controlelrs,  A full update was made
    if (radio.Listen() )
    {
	    strip.Paint();
    }
}
