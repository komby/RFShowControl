/*
 * WM2999Receiver.cpp
 *
 *  Created on: Mar  2013
 *      Author: Greg Scull, komby@komby.com
 *
 *      This code is a derivative of the original work done by
 *      Joe Johnson RFColor_24 Receiver Program
 *
 *      The Code which Joe wrote inspired this software, the related hardware and
 *      it was also used as a starting point for this class.
 *
 *      As with the RFColor_24 The commercial Use of this software is Prohibited.
 *      Use this software and (especially) the hardware at your own risk.
 *
 *      !!!!WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING !!!!
 *
 *      The WM2999 controller is a HIGH VOLTAGE DEVICE
 *
 *      DO NOT, I REPEAT... DO NOT TOUCH, LICK, or otherwise MOLEST THE WM2999 Controller
 *      OR the Original Holiday time color changing lights controller while it is plugged in a power outlet.
 *      Without proper isolation hardware you MUST not connect the controller
 *      to your computer while it is plugged in.
 *
 *      Users of this software agree to hold harmless the creators and contributors
 *      of this software.  By using this software you agree that you are doing so at your own risk, you
 *      could kill yourself or someone else by using this software and/or modifying the
 *      factory controller.  You, by using this software, are assuming all legal responsibility
 *      for the use of the software and any hardware it is used on.
 *
 *      The Commercial Use of this Software is Prohibited.
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <WM2999PixelControl.h>
#include <IPixelControl.h>
#include <WM2999.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

// REQUIRED VARIABLES
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE  RF1_1_3
#define PIXEL_TYPE			WM_2999
#define PIXEL_DATA_PIN			A0

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_1MBPS
#define LISTEN_CHANNEL 100	// the channel for the RF Radio



// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

// If you're not using Over-The-Air configuration these variables are required:
#define HARDCODED_START_CHANNEL 0
#define HARDCODED_NUM_PIXELS 20

//Uncomment for serial
#define DEBUG 0

//NRF24L01+ Items

//#define outPin 19  //Arduino pin # that Lights are Connected to.  This is actually Pin #28 on the Atmega 328 IC
#define lightCount 20  //Total # of lights on string (usually 50, 48, or 36)




//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>


//Arduino setup function.
void setup() {
 printf_begin();
 
 
	Serial.begin(57600);
	Serial.write("Initializing reciever\n");
Serial.write(A0);	//The WM2999 Light string data line is connected to this pin.
	pinMode(A0, OUTPUT);
	digitalWrite(A0,LOW);
	delay(2);
	strip.SetPixelCount(20);
	strip.Paint();

	Serial.write("Initializing Radio\n");
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	
	uint8_t logicalControllerNumber = 0;
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		
		radio.AddLogicalController(logicalControllerNumber, HARDCODED_START_CHANNEL, HARDCODED_NUM_PIXELS * 3,  RECEIVER_UNIQUE_ID);
	}
	
	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
  radio.printDetails(); 
	Serial.write("Init and Paint LEDS for startup \n");
	
	//Both OTA and NON ota will need to set their data base pointers.
	logicalControllerNumber = 0;
	strip.Begin(radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber));
	
	
    radio.DisplayDiagnosticStartup(&strip) ;
}
      



//RF Listening to DMX packets loop
void loop(void){

	//When Radio.Listen returns true its time to update
	if (radio.Listen() )
	{
		strip.Paint();
	}
}

