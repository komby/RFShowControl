/*
 * OTA Manual Configuration
 *
 *    Input: Manual entry
 *    Output: OTA Configuration Packets
 *
 * Author: Mat Mrosko, rfpixelcontrol@matmrosko.com
 * Date: November 25, 2015
 *		New Code
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

#include <printf.h>

#include "helpers.h"

/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        RF1
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
// NOTE Receivers programmed after October 2015 should leave the DATA_RATE at RF24_250KBPS,  Receivers programmed prior to October 2015 ( beta 4 code) need to set this data rate
//to the data rate of the target receiver.
#define DATA_RATE                       RF24_250KBPS
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
#define DEBUG                           1

// PACKET_SEND_DELAY Description: http://learn.komby.com/wiki/58/configuration-settings#PACKET_SEND_DELAY
#define PACKET_SEND_DELAY               1000
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
#define RF_WRAPPER                      1
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"

#define MAX_LOGICAL_CONTROLLERS         5

void setup(void)
{
#ifdef DEBUG
  Serial.begin(115200);
  printf_begin();
  printf("Startup\n");
#endif

  radio.Initialize( radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, DATA_RATE);
  delay(1000);

#ifdef DEBUG
  radio.printDetails();
#endif
}

void loop(void)
{
	sendInitPacket(1, 88, RF24_1MBPS, 1);
	send_LED_OTA_Packet(1, 361, 150);

	delay(100);

//	sendInitPacket(9, 88, RF24_1MBPS, 1);
//	send_DMX_OTA_Packet(9, 1, 32);
//
//	delay(100);
//
//	sendInitPacket(10, 88, RF24_1MBPS, 1);
//	send_DMX_OTA_Packet(10, 33, 32);
//
//	delay(100);
//
//	sendInitPacket(11, 88, RF24_1MBPS, 1);
//	send_DMX_OTA_Packet(11, 65, 64);
//
//	delay(100);
//
//	sendInitPacket(12, 88, RF24_1MBPS, 1);
//	send_DMX_OTA_Packet(12, 100, 64);
//
//	delay(100);
//
//	sendInitPacket(13, 88, RF24_1MBPS, 1);
//	send_DMX_OTA_Packet(13, 135, 64);
//
//	delay(100):
}
