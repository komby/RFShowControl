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
#include <EEPROM.h>
/**
 * The LPD6803 receiver is only designed to control 1 string of lights at a time.
 * 
 * NOTE:  I do not have any LPD6803 pixels to test this with I simply made sure it looked like the code would work. 
 * Use at your own risk.
 * As the receiver is listening to a Transmitter capable of transmitting 512 channels
 * We need to make sure that we are listening to the right channels in the universe
 */
 
 
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup. 
// If you are not using Over The air Configuration you do not need to change this setting.
//Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values: RFCOLOR2_4, MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF
//Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
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
 #define HARDCODED_START_CHANNEL 1
 #define HARDCODED_NUM_PIXELS 11  //This defines the # of LED Channels. ( bug that makes you set this one more than you need  

//What RF Channel do you want to listen to?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	


/***************************  END CONFIGURATION SECTION *************************************************/

#define RECEIVER_NODE 1
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

uint8_t* data;

//Arduino setup function.
void setup() {
  Serial.begin(57600);
  printf_begin();
  strip.SetPixelCount(HARDCODED_NUM_PIXELS );

  Serial.write("Initializing Radio\n");

 radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
 uint8_t logicalControllerNumber = 0;
 #if (OVER_THE_AIR_CONFIG_ENABLE == 0)
 
 radio.AddLogicalController(logicalControllerNumber++, HARDCODED_START_CHANNEL, HARDCODED_NUM_PIXELS*3, 0);
 #endif

 radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);
 radio.printDetails();
 //radio.PrintControllerConfig();

 logicalControllerNumber = 0;
  data = radio.GetControllerDataBase(logicalControllerNumber);
 strip.Begin(data, radio.GetNumberOfChannels(logicalControllerNumber));
strip.setCPUmax(100);
  Serial.write("Init and Paint LEDS for startup \n");
  delay (2);
  for(int i=0;i<strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 0,0,0);
  strip.Paint();
  delay(2000);
  for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i,255,255,255);
  strip.Paint();
  delay(2000);
  for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 0,0,0);
  strip.Paint();  
  delay(2000);
  for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 255,0,0);
  strip.Paint();
  delay(2000);
    for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 0,255,0);
  strip.Paint();
  delay(2000);
 for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 0,0,255);
  strip.Paint();
  delay(2000);
  for(int i=0;i<=strip.GetPixelCount(); i++)
  strip.SetPixelColor(i, 0,0,0);
  strip.Paint();  
  delay(2000);
}




void loop(void){
  if (radio.Listen() )
  {
          noInterrupts();
	  for(int i=1;i<strip.GetPixelCount(); i++)
	  {  
              // critical, time-sensitive code here
               // other code here
                 int offset = i*3;
		 strip.SetPixelColor(i, data[(offset)], data[(offset)+1], data[offset+2]);
               //  strip.Paint();
	  }
          
          interrupts();
	  strip.Paint();
         // delay(1);
  }
}



