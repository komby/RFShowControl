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
// REQUIRED VARIABLES
#define RECEIVER_UNIQUE_ID 50
#define NRF_TYPE			RF1_1_3
#define PIXEL_TYPE			FAST_SPI
#define PIXEL_PROTOCOL		WS2801
#define PIXEL_DATA_PIN 2
#define PIXEL_CLOCK_PIN 4

// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 1

// If you're not using Over-The-Air configuration these variables are required:
//If you are using OTA then your done,  ignore this stuff.
#define DMX_START_CHANNEL 0
#define DMX_NUM_PIXELS 50
#define LISTEN_CHANNEL 100	// the channel for the RF Radio
#define DATA_RATE RF24_1MBPS

//How Many Pixels are in your string?  This is the number of pixels not the number of LEDs or channels
#define NUM_LEDS 50
#define NUM_LEDS_PER_PIXEL 3


//You dont really need to change these.
//How Bright should our LEDs start at
#define LED_BRIGHTNESS 128 //50%

/**************END CONFIGURATION SECTION ***************************/
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>

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
	
	LEDS.setBrightness(LED_BRIGHTNESS);
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
	delay(2000);
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	
	
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		
		radio.AddLogicalController(RECEIVER_UNIQUE_ID, DMX_START_CHANNEL, NUM_LEDS * NUM_LEDS_PER_PIXEL,  0);
	}
	
	radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,RECEIVER_UNIQUE_ID);
	radio.printDetails();
	LEDS.setBrightness(LED_BRIGHTNESS);
	
	uint8_t logicalControllerNumber = 0;
	
	leds =(CRGB*) radio.GetControllerDataBase(logicalControllerNumber++);
	
	


	#if (PIXEL_PROTOCOL == LPD8806)
	LEDS.addLeds<LPD8806,PIXEL_DATA_PIN, PIXEL_CLOCK_PIN>(leds, NUM_LEDS);
	//LEDS.addLeds<WS2801, 11, 13, BGR, DATA_RATE_MHZ(1)>(leds, NUM_LEDS);
	#elif (PIXEL_PROTOCOL == WS2801)
	LEDS.addLeds(new WS2801Controller<PIXEL_DATA_PIN, PIXEL_CLOCK_PIN, RGB>(), leds, NUM_LEDS, 0);
	//LEDS.addLeds<WS2801, PIXEL_DATA_PIN , PIXEL_CLOCK_PIN , RGB >(leds , NUM_LEDS);
	#elif (PIXEL_PROTOCOL == WS2811)
	LEDS.addLeds<WS2811,PIXEL_DATA_PIN>(leds, NUM_LEDS);
	#elif  (PIXEL_PROTOCOL == UCS1903)
	LEDS.addLeds<UCS1903, PIXEL_DATA_PIN>(leds, NUM_LEDS);
	#elif  (PIXEL_PROTOCOL == TM1803)
	LEDS.addLeds<TM1803, PIXEL_DATA_PIN>(leds, NUM_LEDS);
	#elif  (PIXEL_PROTOCOL == SM16716)
	LEDS.addLeds<SM16716,PIXEL_DATA_PIN>(leds, NUM_LEDS);
	#else Must define PIXEL_PROTOCOL : (WS2801,LPD8806,WS2811,UCS1903,TM1803,SM16716)
	#endif

	
	//Initalize the data for LEDs
	memset(leds, 0,  NUM_LEDS * sizeof(struct CRGB));
	delay (200);
	radio.PrintControllerConfig();
}

void loop(void)
{
	//When Radio.Listen returns true its time to update the LEDs for all controlelrs,  A full update was made
	if (radio.Listen() )
	{
		LEDS.show();
	}
}