/*
 * TODO
 *
 *    Input: nRF
 *    Output: Multiple Pixel Types (configurable below)
 *
 * Created on: May, 28th 2014
 * Author: Mat Mrosko <rfpixelcontrol@matmrosko.com>
 *
 * License:
 *    Users of this software agree to hold harmless the creators and
 *    contributors of this software.  By using this software you agree that
 *    you are doing so at your own risk, you could kill yourself or someone
 *    else by using this software and/or modifying the factory controller.
 *    By using this software you are assuming all legal responsibility for
 *    the use of the software and any hardware it is used on.
 *
 *    The Commercial Use of this Software is Prohibited.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <FastLED.h>
#include <GEColorEffects.h>
#include <LPD6803.h>
#include <TimerOne.h>
#include <WM2999.h>

#include "LPD6803.h"
#include "IRFShowControl.h"
#include "MemoryFree.h"
#include "printf.h"
#include "RFShowControl.h"
#include "WM2999RFShowControl.h"

/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIEILD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1

// PIXEL_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#PIXEL_TYPE
// Valid Values: GECE, LPD_6803, LPD_8806, SM_16716, TM_1803, TM_1809, UCS_1903, WM_2999, WS_2801, WS_2811
#define PIXEL_TYPE                      WS_2811

// PIXEL_COLOR_ORDER Description: http://learn.komby.com/wiki/58/configuration-settings#PIXEL_COLOR_ORDER
// Valid Values: RGB, RBG, GRB, GBR, BRG, BGR
#define PIXEL_COLOR_ORDER               BRG

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/wiki/58/configuration-settings#OVER_THE_AIR_CONFIG_ENABLE
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/wiki/58/configuration-settings#RECEIVER_UNIQUE_ID
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33

// CHANNEL_GROUPING_MODE Description: http://learn.komby.com/wiki/58/configuration-settings#CHANNEL_GROUPING_MODE
// Valid Values: SINGLE, PIXEL  //single treats as an individual channel, PIXEL Groups by 3s
#define CHANNEL_GROUPING_MODE PIXEL

// CHANNEL_REPEAT_COUNT Description: http://learn.komby.com/wiki/58/configuration-settings#CHANNEL_REPEAT_COUNT
// Valid Values: 1-512 //use this to repeat a channel based on channel grouping mode
#define CHANNEL_REPEAT_COUNT 5


/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description:  http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL Description:  http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_START_CHANNEL
// Valid Values: 1-512
//#define HARDCODED_START_CHANNEL         1
#define HARDCODED_START_CHANNEL_1         101  
#define HARDCODED_START_CHANNEL_2         161
const uint8_t logicalControllerStartChannels[] = {HARDCODED_START_CHANNEL_1, HARDCODED_START_CHANNEL_2 };

// HARDCODED_NUM_PIXELS Description:  http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_PIXELS
// Valid Values: 1-170
#define HARDCODED_NUM_PIXELS_1            20
#define HARDCODED_NUM_PIXELS_2            20

#define FAST_SPI_CONTROL 1
// values must be evenly divisible by their respective start channel from the previous setting

#define HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_1 HARDCODED_NUM_PIXELS_1 / CHANNEL_REPEAT_COUNT
#define HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_2 HARDCODED_NUM_PIXELS_2 / CHANNEL_REPEAT_COUNT
const uint8_t logicalControllersNumPixelsPer[] = {HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_1, HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_2};

/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// NUMBER_LOGICAL_CONTROLLERS Description http://learn.komby.com/wiki/58/configuration-settings#NUMBER_LOGICAL_CONTROLLERS
// Valid Values: 1-3 (for now)
#define NUMBER_LOGICAL_CONTROLLERS 2

// PIXEL_DATA_PIN Description:  http://learn.komby.com/wiki/58/configuration-settings#PIXEL_DATA_PIN
// Valid Values: List of arduino Analog or Digital pins used for data signal, typically ~1-16 
#define PIXEL_DATA_PIN_1                  2
#define PIXEL_DATA_PIN_2                  6
const uint8_t pixelDataPins[] = {PIXEL_DATA_PIN_1, PIXEL_DATA_PIN_2};

// PIXEL_CLOCK_PIN Description:  http://learn.komby.com/wiki/58/configuration-settings#PIXEL_CLOCK_PIN
// Valid Values: List of arduino Analog or Digital pin(s) used for clock signal on 4 wire clocked pixels, typically ~1-16
#define PIXEL_CLOCK_PIN_1                 4
#define PIXEL_CLOCK_PIN_2                 3
const uint8_t pixelClockPins[] = { PIXEL_CLOCK_PIN_1, PIXEL_CLOCK_PIN_2 };


// LOGICAL_DATA_ORDER Description:  http://learn.komby.com/wiki/58/configuration-settings#LOGICAL_DATA_ORDER
// Valid Values: 0 or 1 - Setting to 1 will invert the output order.
#define LOGICAL_DATA_ORDER_1 1
#define LOGICAL_DATA_ORDER_2 0

//how Bright should our LEDs start at Description:  http://learn.komby.com/wiki/58/configuration-settings#LED_BRIGHTNESS
#define LED_BRIGHTNESS                  128 //50%

#define DEBUG                           1
uint8_t* data ;
//FCC_RESTRICT Description: http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT
//Valid Values: 1, 0  (1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

//Arduino setup function.
void setup(void)
{
#ifdef DEBUG
  Serial.begin(115200);
  printf_begin();
  Serial.println("Initializing Radio");
#endif

  radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
  uint8_t logicalControllerNumber = 0;
  if(!OVER_THE_AIR_CONFIG_ENABLE)
  {
        //for ( int nLogical = 0; nLogical < NUMBER_LOGICAL_CONTROLLERS; nLogical++){
	    printf("adding logical Controller\n");
            radio.AddLogicalController(0, HARDCODED_START_CHANNEL_1, HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_1 * 3, 0);
            radio.AddLogicalController(1, HARDCODED_START_CHANNEL_2, HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_2 * 3, 0);
	//}
  }
  
  

  radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);

#ifdef DEBUG
  radio.printDetails();
  Serial.print(F("PixelColorOrder: "));
  printf("%d\n", PIXEL_COLOR_ORDER);
#endif

  logicalControllerNumber = 0;
  int countOfPixels = radio.GetNumberOfChannels(0)/3;

#ifdef DEBUG
  Serial.print(F("Number of channels configured "));
  printf("%d\n", countOfPixels);
#endif


#ifdef FAST_SPI_CONTROL
    leds = (CRGB*) (uint8_t*)calloc(HARDCODED_NUM_PIXELS_1 + HARDCODED_NUM_PIXELS_2,sizeof(CRGB));

data = radio.GetControllerDataBase(0);

  //Initalize the data for LEDs
  //todo eventually this will be a bug
  delay(200);
  LEDS.setBrightness(LED_BRIGHTNESS);
//#else
 // strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber));
//
 // for (int i = 0; i < strip.GetElementCount() / 3; i++)
//  {
//    strip.SetElementColor(i, strip.Color(0, 0, 0));
//  }
//  strip.Paint();
#endif

  #if (PIXEL_TYPE == LPD_8806)
  LEDS.addLeds(new LPD8806Controller<PIXEL_DATA_PIN_1, PIXEL_CLOCK_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new LPD8806Controller<PIXEL_DATA_PIN_2, PIXEL_CLOCK_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == WS_2801)
  LEDS.addLeds(new WS2801Controller<PIXEL_DATA_PIN_1, PIXEL_CLOCK_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new WS2801Controller<PIXEL_DATA_PIN_2, PIXEL_CLOCK_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == SM_16716)
  LEDS.addLeds(new SM16716Controller<PIXEL_DATA_PIN_1, PIXEL_CLOCK_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new SM16716Controller<PIXEL_DATA_PIN_1, PIXEL_CLOCK_PIN_1, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == TM_1809)
  LEDS.addLeds(new TM1809Controller800Khz<PIXEL_DATA_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
   LEDS.addLeds(new TM1809Controller800Khz<PIXEL_DATA_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == TM_1803)
  LEDS.addLeds(new TM1803Controller400Khz<PIXEL_DATA_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new TM1803Controller400Khz<PIXEL_DATA_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == UCS_1903)
  LEDS.addLeds(new UCS1903Controller400Khz<PIXEL_DATA_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new UCS1903Controller400Khz<PIXEL_DATA_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif (PIXEL_TYPE == WS_2811)
  LEDS.addLeds(new WS2811Controller800Khz<PIXEL_DATA_PIN_1, PIXEL_COLOR_ORDER>(), leds, HARDCODED_NUM_PIXELS_1, 0);
  LEDS.addLeds(new WS2811Controller800Khz<PIXEL_DATA_PIN_2, PIXEL_COLOR_ORDER>(), leds +  HARDCODED_NUM_PIXELS_1, HARDCODED_NUM_PIXELS_2, 0);
  #elif ((PIXEL_TYPE != LPD_6803) && \
       (PIXEL_TYPE != WM_2999) && \
       (PIXEL_TYPE != GECE))
    #error Must define PIXEL_TYPE: (WS_2801,LPD_8806,WS_2811,UCS_1903,TM_1803,SM_16716,LPD_6803,WM_2999,GECE)
  #endif

#ifdef DEBUG
  radio.PrintControllerConfig();
  Serial.print(F("freeMemory()="));
  Serial.println(freeMemory());
#endif
}

void loop(void)
{
  //When Radio.Listen returns true its time to update the LEDs for all controllers, a full update was made
  if (radio.Listen())
  {
#ifdef FAST_SPI_CONTROL
  int pixelsNonDuplicated = HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_1+HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_2;
  int channelRangeStartLogical1 = 0;
  int channelRangeEndLogical1 = channelRangeStartLogical1 + HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_1 * CHANNEL_REPEAT_COUNT;
  
  int channelRangeStartLogical2 = channelRangeEndLogical1;
  int channelRangeEndLogical2 = channelRangeStartLogical2 + HARDCODED_LOGICAL_CONTROLLER_PIXELS_PER_2 * CHANNEL_REPEAT_COUNT;
  

  int pixelsDuplicated = CHANNEL_REPEAT_COUNT * pixelsNonDuplicated;
  
    //if ( CHANNEL_REPEAT_COUNT > 1 ) {
      CRGB* temp = (CRGB*)data;
      memset(leds, 0,  pixelsDuplicated* sizeof(struct CRGB) );
  int j=0;
  
  
      for( int i = 0; i < pixelsDuplicated; i++){
           
          if (i>=CHANNEL_REPEAT_COUNT && (i % CHANNEL_REPEAT_COUNT) ==0)
             j++;        
             //for(int grouping = 0; grouping< 5; grouping++){
//           leds[i] = CRGB(data[j], data[j+1], data[j+2]);
           if ( LOGICAL_DATA_ORDER_1 && i < channelRangeEndLogical1){
             //reverse the order we copy into leds
             leds[(channelRangeEndLogical1- 1 -i)] = temp[j];  
           }
//           else if (LOGICAL_DATA_ORDER_2 && i > channelRangeEndLogical2){
//              leds[(channelRangeEndLogical2- 1 -i)] = temp[j];  
//           }
           else {
           leds[i] = temp[j];
           }
        }
        //duplicate pixel data before passing to leds
    
    //}
   // }

    LEDS.show();
#else
    strip.Paint();
#endif
  }
}