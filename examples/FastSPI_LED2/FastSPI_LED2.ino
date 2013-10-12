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
#include <EEPROM.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include "FastSPI_LED2.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#include <MemoryFree.h>
//
/**************CONFIGURATION SECTION ***************************/
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup. 
// If you are not using Over The air Configuration you do not need to change this setting.
//Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values: RFCOLOR2_4, MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF
#define NRF_TYPE  RF1_1_3

//What Kind of pixels? Valid Values: LPD_8806,WS_2801,SM_16716,TM_1809, TM_1803, UCS_1903, WS_2811
#define PIXEL_PROTOCOL		WS_2811
#define PIXEL_DATA_PIN 2
#define PIXEL_CLOCK_PIN 4

//What order does your pixel string expect the data to be?  
//Valid Values: RGB, RBG,GRB,GBR,BRG,BGR
#define PIXEL_COLOR_ORDER RGB

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

// If you're not using Over-The-Air configuration these variables are required:
//If you are using OTA then your done,  ignore this stuff.
#define HARDCODED_START_CHANNEL 1
#define HARDCODED_NUM_PIXELS 50

//What RF Channel do you want to listen on?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	



/**************END CONFIGURATION SECTION ***************************/
#define PIXEL_TYPE			FAST_SPI
//You dont really need to change these.
//How Bright should our LEDs start at
#define LED_BRIGHTNESS 128 //50%

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
	
	uint8_t logicalControllerNumber = 0;
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		
		radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_PIXELS * 3,  0);
	}
	
	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
	radio.printDetails();
	LEDS.setBrightness(LED_BRIGHTNESS);
	Serial.print(F("PixelColorOrder: "));
	printf("%d\n", PIXEL_COLOR_ORDER);
	logicalControllerNumber = 0;
	
	leds =(CRGB*) radio.GetControllerDataBase(logicalControllerNumber++);
	 int countOfPixels = radio.GetNumberOfChannels(0)/3;

	Serial.print(F("Number of channels configured "));
	printf("%d\n", countOfPixels);
       

	#if (PIXEL_PROTOCOL == LPD_8806)
	LEDS.addLeds(new LPD8806Controller<PIXEL_DATA_PIN, PIXEL_CLOCK_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	#elif (PIXEL_PROTOCOL == WS_2801)
	LEDS.addLeds(new WS2801Controller<PIXEL_DATA_PIN, PIXEL_CLOCK_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	#elif  (PIXEL_PROTOCOL == SM_16716)
	LEDS.addLeds(new SM16716Controller<PIXEL_DATA_PIN, PIXEL_CLOCK_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	#elif  (PIXEL_PROTOCOL == TM_1809)
	LEDS.addLeds(  new TM1809Controller800Khz<PIXEL_DATA_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	#elif  (PIXEL_PROTOCOL == TM_1803)
	LEDS.addLeds(  new TM1803Controller400Khz<PIXEL_DATA_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	#elif  (PIXEL_PROTOCOL == UCS_1903)
	LEDS.addLeds(  new UCS1903Controller400Khz<PIXEL_DATA_PIN, PIXEL_COLOR_ORDER>(), leds,countOfPixels, 0);
	#elif (PIXEL_PROTOCOL == WS_2811)
	LEDS.addLeds(  new WS2811Controller800Khz<PIXEL_DATA_PIN, PIXEL_COLOR_ORDER>(), leds, countOfPixels, 0);
	
	#else Must define PIXEL_PROTOCOL : (WS_2801,LPD_8806,WS_2811,UCS_1903,TM_1803,SM_16716)
	#endif

	
	//Initalize the data for LEDs
	//todo eventually this will be a bug
	memset(leds, 0,  countOfPixels * sizeof(struct CRGB));
	delay (200);
	radio.PrintControllerConfig();
	
	
		Serial.print(F("freeMemory()="));
		Serial.println(freeMemory());
}

void loop(void)
{
	//When Radio.Listen returns true its time to update the LEDs for all controllers,  A full update was made
	if (radio.Listen() )
	{
		LEDS.show();
	}
}