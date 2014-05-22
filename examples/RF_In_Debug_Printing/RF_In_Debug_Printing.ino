/*
 * Print packet content and optionally packet data snooping on transmitter channels
 *
 *		Input: nRF
 *		Output: Serial debug print statements
 *
 * Created on: July 2013
 * Author: Greg Scull, komby@komby.com
 *
 * Updated: May 20, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
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
// Note that this sketch doesn't support OTA.  You can only configure it manually
// to receive data on a known channel and print the output.

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
#define NRF_TYPE					MINIMALIST_SHIELD

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
/*********************** END OF CONFIGURATION SECTION ************************/



/******************** START OF ADVANCED SETTINGS SECTION *********************/
//#define DEBUG						1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

byte data[32];

void setup(void)
{
	printf_begin();
	radio.EnableOverTheAirConfiguration(0);
	radio.AddLogicalController(0, 1, 10, 0);

	Serial.begin(115200);
	Serial.write("Initializing DEBUG listener\n");

	radio.Initialize(radio.RECEIVER, pipes, LISTEN_CHANNEL, DATA_RATE, 0);
	radio.printDetails();

	Serial.write("Init and Paint LEDS for startup \n");
}

void loop(void)
{
	int packetOffset = 0;
	if (radio.available())
	{
		radio.read(&data, 32);

		packetOffset = data[30];
		printf("--%3d - %3d:*****************************\r\n", data[30] * 30, (data[30] * 30) + 30);
		printf("\n\r--%3d:", data[30] * 30);
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
