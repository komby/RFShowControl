/*
 * This sketch is intended to take data from the RF network using a
 * Komby transmitter and control WM2999 pixels.
 *
 *		Input: nRF
 *		Output: WM2999
 *
 * Created: Mar 2013
 * Author: Greg Scull, komby@komby.com
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * This code is a derivative of the original work done by
 * Joe Johnson RFColor_24 Receiver Program
 *
 * The Code which Joe wrote inspired this software, the related hardware and
 * it was also used as a starting point for this class.
 *
 * As with the RFColor_24 The commercial Use of this software is Prohibited.
 * Use this software and (especially) the hardware at your own risk.
 *
 * !!!! WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING !!!!
 *		The WM2999 controller is a HIGH VOLTAGE DEVICE
 *
 * DO NOT, I REPEAT... DO NOT TOUCH, LICK, or otherwise MOLEST THE WM2999
 * Controller OR the Original Holiday time color changing lights controller
 * while it is plugged in a power outlet.  Without proper isolation hardware
 * you MUST not connect the controller to your computer while it is plugged in.
 *
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
#include <WM2999.h>

#include "IPixelControl.h"
#include "printf.h"
#include "RFPixelControl.h"
#include "WM2999PixelControl.h"



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
#define NRF_TYPE					WM_2999_NRF

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
//		http://learn.komby.com/wiki/Configuration:OTA
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
//		http://learn.komby.com/wiki/NON-OTA_vs_OTA

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
//		http://learn.komby.com/wiki/Configuration:Listen_Channel
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
//		http://learn.komby.com/wiki/Configuration:Hardcoded_Start_Channel
#define HARDCODED_START_CHANNEL		1

// HARDCODED_NUM_PIXELS Description:
//		This is the number of pixels this device will be reading the data from the
//		stream and controlling.  This is NOT the number of channels you need.  The
//		actual number of channels is this value times 3, one each for Red, Green,
//		and blue.
//
// Valid Values:
//		1-340
//
// More Information:
//		http://learn.komby.com/wiki/Configuration:Hardcoded_Num_Pixels
#define HARDCODED_NUM_PIXELS		20
/*********************** END OF CONFIGURATION SECTION ************************/



/******************** START OF OTA CONFIGURATION SECTION *********************/
//
// OTA Configuration:
//		If you're not using Over-The-Air configuration, please skip this
//		section, you are done configuring options.
//
// For more information on Non-OTA vs. OTA visit:
//		http://learn.komby.com/wiki/NON-OTA_vs_OTA

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
//		http://learn.komby.com/wiki/Configuration:Receiver_Unique_Id
#define RECEIVER_UNIQUE_ID			33
/********************* END OF OTA CONFIGURATION SECTION **********************/



/******************** START OF ADVANCED SETTINGS SECTION *********************/
//#define DEBUG						1
#define PIXEL_TYPE					WM_2999
#define PIXEL_DATA_PIN				A0

#define HEARTBEAT_PIN				A2
#define RF_LINK_PIN					A1

// Force an update if no data received every TIMEOUT_FORCE_REFRESH ms
#define TIMEOUT_FORCE_REFRESH		200
/********************* END OF ADVANCED SETTINGS SECTION **********************/


//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

int beat = 0;
unsigned long time = millis();

//Arduino setup function.
void setup(void)
{
#ifdef DEBUG
	Serial.begin(57600);
	delay(2);
	Serial.write("Initializing receiver\n");
	printf_begin();
	Serial.write(PIXEL_DATA_PIN);	//The WM2999 Light string data line is connected to this pin.
#endif

	pinMode(PIXEL_DATA_PIN, OUTPUT);
	digitalWrite(PIXEL_DATA_PIN,LOW);

	delay(200);

	pinMode(HEARTBEAT_PIN, OUTPUT);
	pinMode(RF_LINK_PIN, OUTPUT);

#ifdef DEBUG
	Serial.write("Initializing Radio\n");
#endif
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);

	uint8_t logicalControllerNumber = 0;
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_PIXELS * 3, RECEIVER_UNIQUE_ID);
	}
	int link = radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, RECEIVER_UNIQUE_ID);
	digitalWrite(RF_LINK_PIN, link);

#ifdef DEBUG
	radio.printDetails();
	Serial.write("Init and Paint LEDS for startup \n");
	printf("%d num pix\n", radio.GetNumberOfChannels(logicalControllerNumber));
#endif

	//Both OTA and NON ota will need to set their data base pointers.
	logicalControllerNumber = 0;
	strip.WM2999PixelControl::Begin((uint8_t*)radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber)/3);

	for (int i = 0; i<20; i++)
	{
		strip.SetPixelColor(i, strip.Color(0, 255, 0));
	}
	strip.Paint();
	delay(200);
	strip.Paint();
	delay(200);
	strip.Paint();
	delay(200);
	strip.Paint();
	delay(200);

	for (int i = 0; i<20; i++)
	{
		strip.SetPixelColor(i, strip.Color(0, 0, 0));
	}
	strip.Paint();
	time = millis();
}

//RF Listening Loop
void loop(void)
{
	//When Radio.Listen returns true its time to update
	if (radio.Listen())
	{
		strip.Paint();
		time = millis();
		beat = !beat;
		digitalWrite(HEARTBEAT_PIN, beat);
	}
	else if (millis()-time >= TIMEOUT_FORCE_REFRESH)
	{
		strip.Paint();
		time = millis();
	}
}
