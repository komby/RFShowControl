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
// NRF_TYPE Description: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
// Valid Values: RF1, MINIMALIST_SHEILD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE					RF1
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/Configuration:Listen_Channel
// Valid Values: 1-124
#define LISTEN_CHANNEL				100

// DATA_RATE Description: http://learn.komby.com/wiki/Configuration#Data_Rate
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE					RF24_250KBPS
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
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
