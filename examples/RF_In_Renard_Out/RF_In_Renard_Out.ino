/*
 * RenardReceiver
 *
 * Created on: Mar 2013
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
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include "IRFShowControl.h"
#include "RenardControl.h"
#include "RFShowControl.h"
#include <printf.h>

/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, RF1_SERIAL, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        KOMBEE

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/wiki/58/configuration-settings#OVER_THE_AIR_CONFIG_ENABLE
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/wiki/58/configuration-settings#RECEIVER_UNIQUE_ID
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  7

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_1MBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_START_CHANNEL
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL         1

// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_CHANNELS
// Valid Values: 1-512
#define HARDCODED_NUM_CHANNELS          8

// RENARD_BAUD_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#RENARD_BAUD_RATE
// Valid Values: 19200, 38400, 57600, 115200, 230400, 460800
#define RENARD_BAUD_RATE                57600
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1

//FCC_RESTRICT Description: http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT
//Valid Values: 1, 0  (1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      RENARD
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

int beat = 0;
uint8_t * channels;
int numChannels = HARDCODED_NUM_CHANNELS;

#ifndef DEBUG
  int null_out_function(char c, FILE *stream) {return 0;}
#endif

void setup(void)
{
  
#ifndef DEBUG
  fdevopen( &null_out_function, 0 );
#endif

  Serial.begin(RENARD_BAUD_RATE);
#ifdef DEBUG
  printf_begin();
  Serial.println("Initializing Radio");
#endif

  #if (NRF_TYPE==KOMBEE)
  pinMode(HEARTBEAT_PIN, OUTPUT);
  pinMode(HEARTBEAT_PIN_1, OUTPUT);
  #endif
  
  //Setup the tx mode for the RS485 chip
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  
    radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);

  uint8_t logicalControllerNumber = 0;
  if(!OVER_THE_AIR_CONFIG_ENABLE)
  {
    radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_CHANNELS, RENARD_BAUD_RATE);
  }

  radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);
#ifdef DEBUG
  radio.printDetails();
#endif

  logicalControllerNumber = 0;


  channels = radio.GetControllerDataBase(logicalControllerNumber);
  numChannels =  radio.GetNumberOfChannels(logicalControllerNumber);

}

void loop(void)
{
 if (radio.Listen())
 {
   Paint();
   #if (NRF_TYPE==KOMBEE)
     beat=!beat;
     digitalWrite(HEARTBEAT_PIN, beat);  
     digitalWrite(HEARTBEAT_PIN_1, beat);  
   #endif
 }
}


void Paint(void)
{
  Serial.write(0x7E);
  Serial.write(0x80);

  for ( int i = 0; i < (numChannels); i++ )
  {
    switch (channels[i])
    {
      case 0x7D:
        Serial.write(0x7F);
        Serial.write(0x2F);
        break;

      case 0x7E:
        Serial.write(0x7F);
        Serial.write(0x30);
        break;

      case 0x7F:
        Serial.write(0x7F);
        Serial.write(0x31);
        break;

      default:
        Serial.write(channels[i]);
        break;
    }
  }
}
