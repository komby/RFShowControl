/*
 * Print packet content and optionally packet data snooping on transmitter channels
 *
 *    Input: nRF
 *    Output: Serial debug print statements
 *
 * Created on: July 2013
 * Author: Greg Scull, komby@komby.com
 *
 * Updated: May 20, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
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
#include <printf.h>
#include "RFShowControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1

//FCC_RESTRICT Description: http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT
//Valid Values: 1, 0  (1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

byte data[32];
int prev =0;
 int packetOffset = 0;
 
void setup(void)
{
  printf_begin();
  radio.EnableOverTheAirConfiguration(0);
  radio.AddLogicalController(0, 1, 10, 0);

  Serial.begin(115200);
  Serial.write("Initializing DEBUG listener\n");

  radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, 0);
  radio.printDetails();

}

void loop(void)
{
 
  
  if (radio.available())
  {
    radio.read(&data, 32);

    packetOffset = data[30];
   // printf("--%3d - %3d:*****************************\r\n", data[30] * 30, (data[30] * 30) + 30);

    if (packetOffset!=0){
    printf("%3d, ", data[30]);
    }
    else {
       printf("\n\r%3d,", data[30]);
      //printf("");
  }
      prev=packetOffset;
#ifdef DEBUG
    for (int i =0;i<30;i++)
    {
      if ((i-7) % 8 == 0)
      {
        printf("\t\t\n\r--%3d:", i + (packetOffset * 30));
      }
      printf(" 0x%02X", data[i]);
    }
#endif
  }
}