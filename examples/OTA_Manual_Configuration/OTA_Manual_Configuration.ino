/*
 * RFPixelControlConfiguration - Manually configure your OTA nodes.
 *
 *		Input: Not Applicable
 *		Output: OTA Configuration Packets
 *
 * Created on: June 30 2013
 * Updated 3/18/2014 - Added FastLED v2 release
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
#include <nRF24L01.h>
#include <SPI.h>
#include <util.h>

#include "IPixelControl.h"
#include "OTAConfig.h"
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
/********************** END OF REQUIRED CONFIGURATION ************************/



/******************** START OF ADVANCED SETTINGS SECTION *********************/
//#define DEBUG						1
/********************* END OF ADVANCED SETTINGS SECTION **********************/


//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

byte str[32];

void setup(void)
{
#ifdef DEBUG
	Serial.begin(57600);
	printf_begin();
#endif
	radio.Initialize(radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, DATA_RATE, 1);
	delayMicroseconds(5000);

#ifdef DEBUG
	radio.printDetails();
#endif
}

boolean status;

void loop(void)
{
	//ConfigurationDataForReceiver 1
	//
	//COntroller 1 ID 50
	int logicalSequence = 0;
	str[IDX_CONTROLLER_ID] = 50;
	//The First Configuration Packet contains the number of logical controllers for a given controller
	str[IDX_CONFIG_PACKET_TYPE] = CONTROLLERINFOINIT;
	str[IDX_RF_LISTEN_CHANNEL] = 100;
	str[IDX_RF_LISTEN_RATE] = DATA_RATE;
	str[IDX_NUMBER_OF_LOGICAL_CONTROLLERS] = 1;
#ifdef DEBUG
	printf("WritingPayloadFor init packet\n");
#endif
	radio.write_payload(&str[0], 32);
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01)
	{
		status = radio.get_status();
	}

	str[IDX_CONFIG_PACKET_TYPE] =LOGICALCONTROLLER_LED ;
	str[IDX_CONTROLLER_ID] = 50;
	str[IDX_LOGICAL_CONTROLLER_NUMBER] = logicalSequence;
	str[IDX_LOGICAL_CONTROLLER_NUM_CHANNELS] = 3;
	str[IDX_LOGICAL_CONTROLLER_START_CHANNEL] = 1;
#ifdef DEBUG
	printf("WritingPayloadFor config packet\n");
#endif

	radio.write_payload(&str[0], 32);
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01)
	{
		status = radio.get_status();
	}

	//ConfigurationDataForReceiver 2
	//
	//COntroller 2 ID 33
	str[IDX_CONTROLLER_ID] = 33;
	logicalSequence = 0;
	//The First Configuration Packet contains the number of logical controllers for a given controller
	str[IDX_CONFIG_PACKET_TYPE] = CONTROLLERINFOINIT;
	str[IDX_RF_LISTEN_CHANNEL] = 100;
	str[IDX_RF_LISTEN_RATE] = DATA_RATE;
	str[IDX_NUMBER_OF_LOGICAL_CONTROLLERS] = 1;
#ifdef DEBUG
	printf("WritingPayloadFor init packet\n");
#endif
	radio.write_payload(&str[0], 32);
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01)
	{
		status = radio.get_status();
	}

	str[IDX_CONFIG_PACKET_TYPE] = LOGICALCONTROLLER_LED;
	str[IDX_CONTROLLER_ID] = 33;
	str[IDX_LOGICAL_CONTROLLER_NUMBER] = logicalSequence;
	str[IDX_LOGICAL_CONTROLLER_NUM_CHANNELS] = 150;
	str[IDX_LOGICAL_CONTROLLER_START_CHANNEL] = 4;

#ifdef DEBUG
	printf("WritingPayloadFor config packet\n");
#endif

	radio.write_payload(&str[0], 32);
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01)
	{
		status = radio.get_status();
	}
}
