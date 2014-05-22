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
// This configuration section is required whether you're using OTA or not.
//
// If you're using OTA, these values need to be what you configured your
// OTA configuration node to send packets on.
//
// If you're not using OTA, these values are for the actual data coming over
// your wireless komby network.
//

// DATA_RATE Description:
//		If you're using OTA, this most likely needs to be set to RF24_250KBPS.  That
//		is the speed your OTA configuration packets are transmitted at if you didn't
//		customize the OTA configuration sketch.  The OTA packets you send from the
//		configuration web page set the data rate for the RF data your RF transmitter
//		is configured for.
//
//		If you're not using OTA, set this to whatever your RF transmitter sketch was
//		set to when configuring that sketch.
//
// Valid Values:
//		RF24_250KBPS
//		RF24_1MBPS
//
// More Information:
//		http://learn.komby.com/wiki/Configuration:Data_Rate
//
#define DATA_RATE					RF24_250KBPS

// NRF_TYPE Description:
//		What board are you using to connect your nRF24L01+?
//
// Valid Values:
//		RF1 - Most RF1 Devices including RF1_12V devices
//		MINIMALIST_SHIELD - Minimalist shield designed to go atop a standard arduino
//		WM_2999_NRF - WM299 specific board
//		RFCOLOR_2_4 - RFColor24 device to control GECEs
//
// More Information:
//		http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
//
#define NRF_TYPE					RF1

// OVER_THE_AIR_CONFIG_ENABLE Description:
//		If you're using Over-The-Air configuration, set this value to 1 and skip
//		the NON-OTA CONFIGURATION section below.  Otherwise, leave this at 0 and
//		skip the OTA CONFIGURATION section below.
//
//		OTA makes it so you can make an RF node that can re-program your RF1s in
//		the field.  This means the RF1s will search for a configuration broadcast
//		for 5 seconds after power-on.  If no broadcast is found, it will read it's
//		configuration from EEPROM for the last known working configuration.
//
// Valid Values:
//		0 - do not use OTA
//		1 - use OTA
//
// More Information:
//		http://learn.komby.com/Configuration:OTA
//
#define OVER_THE_AIR_CONFIG_ENABLE	0
/********************** END OF REQUIRED CONFIGURATION ************************/



/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
//
// Non-OTA Configuration:
//		If you're using Over-The-Air configuration, please skip to the
//		"OTA Configuration" of the user-customizable options.
//
// For more information on Non-OTA vs. OTA visit:
//		http://learn.komby.com/NON-OTA_vs_OTA

// LISTEN_CHANNEL Description:
//		RF Channel do you want to listen on? This needs to match the channel you've
//		configured your transmitter for broadcasting it's data.  If you're broadcasting
//		multiple universes worth of data, you'll want each transmit/listen channel to be
//		at least X channels away from each other for best results.  There is also a
//		scanner sketch available for your RF1 to log RF activity in the 2.4GHz range
//		to aid in selecting your channels.  Keep in mind, regular WiFi and many other
//		devices are in this range and can cause interference.
//
// Valid Values:
//		1-124
//
// More Information:
//		http://learn.komby.com/Configuration:Listen_Channel
//
#define LISTEN_CHANNEL				100

// HARDCODED_START_CHANNEL Description:
//		This is the start channel for this controller.  If you're transmitting 200
//		channels worth of wireless data and you'd like for this controller to respond
//		to channels 37-43 (2 pixels worth), you would configure HARDCODED_START_CHANNEL
//		to the value 37, and the HARDCODED_NUM_PIXELS to 2.
//
// Valid Values:
//		1-1020
//
// More Information:
//		http://learn.komby.com/Configuration:Hardcoded_Start_Channel
#define HARDCODED_START_CHANNEL		1
/*********************** END OF CONFIGURATION SECTION ************************/



/******************** START OF OTA CONFIGURATION SECTION *********************/
//
// OTA Configuration:
//		If you're not using Over-The-Air configuration, please skip this
//		section, you are done configuring options.
//
// For more information on Non-OTA vs. OTA visit:
//		http://learn.komby.com/NON-OTA_vs_OTA

// RECEIVER_UNIQUE_ID Description:
//		This id should be unique for each receiver in your setup.  This value
//		determines how you will target this node for configuration using the OTA
//		transmit sketch.  You should write this somewhere on the RF1 after
//		programming so in case you need to reprogram the settings, you know which
//		device you're targeting.
//
// Valid Values:
//		1-255
//
// More Information:
//		http://learn.komby.com/Configuration:Receiver_Unique_Id
#define RECEIVER_UNIQUE_ID			33
/********************* END OF OTA CONFIGURATION SECTION **********************/



/******************** START OF ADVANCED SETTINGS SECTION *********************/
//#define DEBUG						1

#define NUM_CHANNELS 3
#define START_CHANNEL 1 //Where in the universe do we start

//Setup for RF1_12V DumbRGB Controller
#define REDPIN 3
#define GREENPIN 5
#define BLUEPIN 9
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

	pinMode(REDPIN, OUTPUT);
	pinMode(GREENPIN, OUTPUT);
	pinMode(BLUEPIN, OUTPUT);

	buffer[0]=255;

	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	uint8_t logicalControllerNumber = 0;
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		 radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, NUM_CHANNELS, 0);
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
	analogWrite(REDPIN, r);
	analogWrite(GREENPIN, g);
	analogWrite(BLUEPIN, b);
	delay(d);
}
