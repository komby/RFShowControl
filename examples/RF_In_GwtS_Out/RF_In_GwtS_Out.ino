/*
 * GWTSReceiver
 *
 *    Input: nRF
 *    Output: Disney's Glow With The Show™
 *
 * Created on: November 25th, 2013
 * Author: Mat Mrosko
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
#include <IPixelControl.h>
#include <nRF24L01.h>
#include <OTAConfig.h>
#include <RF24.h>
#include <RFPixelControl.h>
#include <SPI.h>

#include "printf.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
// Valid Values: RF1, MINIMALIST_SHEILD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/Configuration#OTA
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/Configuration#Receiver_Unique_Id
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/Configuration:Listen_Channel
// Valid Values: 1-124
#define LISTEN_CHANNEL                  100

// DATA_RATE Description: http://learn.komby.com/wiki/Configuration#Data_Rate
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/Configuration#Hardcoded_Start_Channel
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL         1

// HARDCODED_NUM_PIXELS Description: http://learn.komby.com/Configuration#Hardcoded_Num_Pixels
// Valid Values: 1-170
#define HARDCODED_NUM_PIXELS            2
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// PIXEL_DATA_PIN Description: http://learn.komby.com/wiki/Configuration#Pixel_Data_Pin
// Valid Values: Any arduino Analog or Digital pin, typically ~1-16
#define PIXEL_DATA_PIN                  2

//#define DEBUG                           1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      GWTS_EARS
//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

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
    radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_PIXELS * 3, 0);
  }

  radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);

#ifdef DEBUG
  radio.printDetails();
#endif

  logicalControllerNumber = 0;
  strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber));
}

void loop(void)
{
  if (radio.Listen())
  {
    strip.Paint();
  }
}
