/*
 * RF1DumbRGB - RFShowControl Receiver code for the DumbRGB expansion board.
 *
 * Created on: Mar 2013
 * Updated 6/2/2013
 * Author: Greg Scull, komby@komby.com
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
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
#include <printf.h>

#include "IRFShowControl.h"
#include "RFShowControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/wiki/58/configuration-settings#OVER_THE_AIR_CONFIG_ENABLE
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/wiki/58/configuration-settings#RECEIVER_UNIQUE_ID
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33

//RGB_BOARD_TYPE Description  http://learn.komby.com/wiki/58/configuration-settings#RGB_BOARD_TYPE
//Valid Values: DUMB_RGB, FLOODUINO
#define RGB_BOARD_TYPE DUMB_RGB
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_START_CHANNEL
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL         1


/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_CHANNELS
// Valid Values: 3, 4
#if (RGB_BOARD_TYPE == DUMB_RGB)
#define HARDCODED_NUM_CHANNELS          3
#elif (RGB_BOARD_TYPE == FLOODUINO)
#define HARDCODED_NUM_CHANNELS			4
#endif


// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1


// Jrd: He is using 9, 3, 10, 5 RGBW respectively


// RGB Pin Descriptions: http://learn.komby.com/wiki/58/configuration-settings#RGB-Pins
#if (RGB_BOARD_TYPE == DUMB_RGB)
	#define RED_PIN                         3
	#define GREEN_PIN                       5
	#define BLUE_PIN                        9	
#elif (RGB_BOARD_TYPE  == FLOODUINO)
	#define RED_PIN                         9
	#define GREEN_PIN                       3
	#define BLUE_PIN                        10
	#define WHITE_PIN                       5
#endif



//FCC_RESTRICT Description: http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT
//Valid Values: 1, 0  (1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

byte *buffer;
void dumbRGBShow(int r, int g, int b, int w, int d);

void setup(void)
{
#ifdef DEBUG
  Serial.begin(115200);
  printf_begin();
#endif

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

#ifdef  WHITE_PIN 
  pinMode(WHITE_PIN, OUTPUT);
#endif

  buffer[0]=255;

  radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
  uint8_t logicalControllerNumber = 0;
  if(!OVER_THE_AIR_CONFIG_ENABLE)
  {
     radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_CHANNELS, 0);
  }

   delay(2);

  radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);
#ifdef DEBUG
  radio.printDetails();
#endif

  buffer= radio.GetControllerDataBase(0);
  delay(200);

  dumbRGBShow(255, 0, 0, 0, 25);
  delay(200);
  dumbRGBShow(0, 255, 0, 0, 25);
  delay(200);
  dumbRGBShow(0, 0, 255, 0, 25);
  delay(200);
  dumbRGBShow(0, 0, 0, 0, 25);
}

void loop(void)
{
  if (radio.Listen())
  {
	  
	  #ifdef  WHITE_PIN 
	  dumbRGBShow(buffer[0], buffer[1], buffer[2], buffer[3],1);  
	  #else
		dumbRGBShow(buffer[0], buffer[1], buffer[2], 0, 1);  
	  #endif
    
  }
}


void dumbRGBShow(int r, int g, int b, int w, int d)
{
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
  #ifdef  WHITE_PIN 
	analogWrite(WHITE_PIN, b);
  #endif
  delay(d);
}