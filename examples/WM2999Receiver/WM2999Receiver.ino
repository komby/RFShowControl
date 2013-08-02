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


/***************************  CONFIGURATION SECTION *************************************************/
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup. 
// If you are not using Over The air Configuration you do not need to change this setting.
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

//What board are you using to connect your nRF24L01+?
//Valid Values:  MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF, RFCOLOR2_4
#define NRF_TYPE  WM_2999_NRF

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to listen on?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	

// Set OTA_CONFIG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for 5 seconds after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

// If you're not using Over-The-Air configuration these variables are required:
#define HARDCODED_START_CHANNEL 1
#define HARDCODED_NUM_PIXELS 20


/*************************** END CONFIGURATION SECTION *************************************************/
//Uncomment for serial
#define DEBUG 1
#define PIXEL_TYPE			WM_2999
#define PIXEL_DATA_PIN			A0

//#define outPin 19  //Arduino pin # that Lights are Connected to.  This is actually Pin #28 on the Atmega 328 IC
#define lightCount 20  //Total # of lights on string (usually 50, 48, or 36)

//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>

//Arduino setup function.
void setup() {

	
	Serial.begin(57600);
   
	delay(2);
	Serial.write("Initializing receiver\n");
     printf_begin();
	Serial.write(A0);	//The WM2999 Light string data line is connected to this pin.
	pinMode(A0, OUTPUT);
	digitalWrite(A0,LOW);

	delay(2);

	//strip.SetPixelCount(20);

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
      //  uint8_t* ptr = radio.GetControllerDataBase(logicalControllerNumber);
        printf("%d num pix\n", radio.GetNumberOfChannels(logicalControllerNumber));
	strip.WM2999PixelControl::Begin((uint8_t*)radio.GetControllerDataBase(logicalControllerNumber), radio.GetNumberOfChannels(logicalControllerNumber)/3);
for ( int i = 0; i<20;i++){
strip.SetPixelColor(i, strip.Color(244,0,255));
}
	strip.Paint();
delay (5000);
for ( int i = 0; i<20;i++){
      strip.SetPixelColor(i, strip.Color(0,255,0));
}
strip.Paint();
delay (300);

    //radio.DisplayDiagnosticStartup(&strip) ;
}


//RF Listening Loop
void loop(void){

	//When Radio.Listen returns true its time to update
	if (radio.Listen() )
	{
		strip.Paint();
	}
}

