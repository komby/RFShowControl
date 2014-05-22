/*
 * SerialDMXToRFTransmitter
 *
 *		This code is a derivative of the original work done by
 *		Joe Johnson RFColor_24 Receiver Program
 *
 *		The Code which Joe wrote inspired this software, the related hardware and
 *		it was also used as a starting point for this class.
 *
 *		As with the RFColor_24 The commercial Use of this software is Prohibited.
 *		Use this software and (especially) the hardware at your own risk.
 *
 *		This code also uses a modified version of the DMXSerial library @see ModifiedDMXSerial for more information.
 *
 *		NOTE: This no longer requries the use of the hardware serial hack
 *
 * Created on: Mar 2014
 * Author: Greg Scull, komby@komby.com
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

#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include "ModifiedDMXSerial.h"
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


//
// Non-OTA Configuration:
//		If you're using Over-The-Air configuration, please skip to the
//		"OTA Configuration" of the user-customizable options.
//
// For more information on Non-OTA vs. OTA visit:
//		http://learn.komby.com/wiki/NON-OTA_vs_OTA

// TRANSMIT_CHANNEL Description:
//		RF Channel do you want to transmit on? This needs to match the channel you've
//		configured your receivers for listening for data.  If you're broadcasting
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
//		http://learn.komby.com/wiki/Configuration:Transmit_Channel
//
#define TRANSMIT_CHANNEL				100
/********************** END OF REQUIRED CONFIGURATION ************************/


/******************** START OF ADVANCED SETTINGS SECTION *********************/
//#define DEBUG						1

//You can listen and transmit less, but why would you?
#define DMX_NUM_CHANNELS			512
/********************* END OF ADVANCED SETTINGS SECTION **********************/

//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"

void setup(void)
{
	radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL,DATA_RATE, 0);
	ModifiedDMXSerial.maxChannel(DMX_NUM_CHANNELS);
	ModifiedDMXSerial.init(DMXReceiver);
}

void loop(void)
{
	if (ModifiedDMXSerial.isPacketReady())
	{
		radio.write_payload(ModifiedDMXSerial.GetPacketPointer(), 32);
		ModifiedDMXSerial.setPacketReady(false);
	}
}
