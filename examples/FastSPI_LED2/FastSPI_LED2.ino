/*
* FastSPI_LED2Receiver  RFPixel Control Receiver Sketch for handling the FAST_SPI2 Release candidate.
*
*  Created on: Mar  2013
*  Updated 7/6/2013
*      Author: Greg Scull, komby@komby.com
*
*      Users of this software agree to hold harmless the creators and contributors
*      of this software. You, by using this software, are assuming all legal responsibility
*      for the use of the software and any hardware it is used on.
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
#define OVER_THE_AIR_CONFIG_ENABLE 1

//Step 0.  Over The Air Configuration Enabled
//Make this number unique for every receiver that you enable
//the OTA configuration will need it
#define RECEIVER_UNIQUE_ID 33

//change the 1 to a 0 if you do not want
//to use OTA configuration
// #define  OVER_THE_AIR_CONFIGURATION 1

//Step 1.
//Radio Configuration -
//What Channel do you want this nRF24L01+ to listen on?
//will be used for fallback if EEPROM and OTA do not work
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

//struct CRGB leds[NUM_LEDS];
//Step 4.
//How Bright should our LEDs start at
#define LED_BRIGHTNESS 128 //50%

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


CRGB* leds;

//Arduino setup function.
void setup()
{
	Serial.begin(57600);
	
	printf_begin();
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
	delay(2000);
	 radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIGURATION);
	 
	 
	 if(!OVER_THE_AIR_CONFIGURATION)
	 {
		 radio.AddLogicalController(RECEIVER_UNIQUE_ID, DMX_START_CHANNEL, NUM_LEDS * NUM_LEDS_PER_PIXEL,  0);
	 }
	 
	radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,RECEIVER_UNIQUE_ID);
	radio.printDetails();
	LEDS.setBrightness(LED_BRIGHTNESS);
	
	uint8_t logicalControllerNumber = 0;
	
	leds =(CRGB*) radio.GetControllerDataBase(logicalControllerNumber++);
	
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

void loop(void)
{
	//When Radio.Listen returns true its time to update the LEDs for all controlelrs,  A full update was made
	if (radio.Listen() )
	{
		LEDS.show();
	}
}
