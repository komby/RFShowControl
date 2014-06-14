/*
 * RF1DumbRGB - RFPixelControl Receiver code for the DumbRGB expansion board.
 * 
 * Created on: Mar 2013
 * Updated 6/2/2013
 * Author: Greg Scull, komby@komby.com
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * License:
 *		Users of this software agree to hold harmless the creators and
 *		contributors of this software.  By using this software you agree that
 *		you are doing so at your own risk, you could kill yourself or someone
 *		else by using this software and/or modifying the factory controller.
 *		By using this software you are assuming all legal responsibility for
 *		the use of the software and any hardware it is used on.
 *
 *		The Commercial Use of this Software is Prohibited.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include "IPixelControl.h"
#include "printf.h"
#include "RFPixelControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
// Valid Values: RF1, MINIMALIST_SHEILD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE					RF1

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/Configuration#OTA
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE	0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/Configuration#Receiver_Unique_Id
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID			33
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/Configuration:Listen_Channel
// Valid Values: 1-124
#define LISTEN_CHANNEL				100

// DATA_RATE Description: http://learn.komby.com/wiki/Configuration#Data_Rate
// Valid Values: RF24_250KBPS, RF24_1MBPS 
#define DATA_RATE					RF24_250KBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/Configuration#Hardcoded_Start_Channel
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL		1

// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/Configuration#Hardcoded_Num_Channels
// Valid Values: 1-512
#define HARDCODED_NUM_CHANNELS		512
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
//#define DEBUG						1

//Setup for RF1_12V DumbRGB Controller
#define RED_PIN						3
#define GREEN_PIN					5
#define BLUE_PIN					9
/********************* END OF ADVANCED SETTINGS SECTION **********************/


//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

byte *buffer;
void dumbRGBShow(int r, int g, int b, int d);

void setup(void)
{
#ifdef DEBUG
	Serial.begin(57600);
	printf_begin();
#endif

	pinMode(RED_PIN, OUTPUT);
	pinMode(GREEN_PIN, OUTPUT);
	pinMode(BLUE_PIN, OUTPUT);

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

	dumbRGBShow(255, 0, 0, 25);
	delay(200);
	dumbRGBShow(0, 255, 0, 25);
	delay(200);
	dumbRGBShow(0, 0, 255, 25);
	delay(200);
	dumbRGBShow(0, 0, 0, 25);
}

void loop(void)
{
	if (radio.Listen())
	{
		dumbRGBShow(buffer[0], buffer[1], buffer[2], 1);
	}
}
 
void dumbRGBShow(int r, int g, int b, int d)
{
	analogWrite(RED_PIN, r);
	analogWrite(GREEN_PIN, g);
	analogWrite(BLUE_PIN, b);
	delay(d);
}
