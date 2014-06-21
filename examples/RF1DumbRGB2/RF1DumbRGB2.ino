/*
 * RF1DumbRGB.ino
 * RFPixelControl Receiver code to handle a RF1_12V with connected DumbRGB expansion board.
 * 
 *  Created on: Mar  2013
 *  Updated 6/2/2013
 *      Author: Greg Scull, komby@komby.com
 *      
 */


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include "printf.h"


/*************************** CONFIGURATION SECTION *************************************************/
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup. 
// If you are not using Over The air Configuration you do not need to change this setting.
//Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

 //What board are you using to connect your nRF24L01+?
 //Valid Values:  MINIMALIST_SHIELD, RF1,KOMBEE, 
 //Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
 #define NRF_TYPE  MINIMALIST_SHIELD

// Set OVER_THE_AIR_CONFIG_ENABLEG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for a short period after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

#define NUM_CHANNELS 3  
#define START_CHANNEL 1 //Where in the universe do we start



//What RF Channel do you want to listen on?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS


//Setup for RF1_12V DumbRGB Controller
#define REDPIN 9
#define GREENPIN 3
#define BLUEPIN 5
#define WHITEPIN 10
 
 /**************END CONFIGURATION SECTION ***************************/
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>
#define RECEIVER_NODE 1
bool readytoupdate=false;

byte * buffer;


//Uncomment for serial
#define DEBUG 0


//Arduino setup function.
void setup() {
	Serial.begin(57600);
	pinMode(REDPIN, OUTPUT);
	pinMode(GREENPIN, OUTPUT);
	pinMode(BLUEPIN, OUTPUT);
          
	buffer[0]=255;
   
    
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
         int logicalControllerSequenceNum = 0;
         radio.AddLogicalController(logicalControllerSequenceNum, START_CHANNEL, NUM_CHANNELS,0);
	}
	printf_begin();
	
   	delay(2);

     radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
      radio.printDetails(); 
      //initalize data buffer
      buffer= radio.GetControllerDataBase(0);
	delay (200);

	  ///This is just start test of channels
	dumbRGBShow(255, 0, 0, 0, 25);
	delay (200);
	dumbRGBShow(0, 255, 0, 0, 25);
	delay (200);
	dumbRGBShow(0, 0, 255, 0, 25);
	delay (200);
	dumbRGBShow(0, 0,  0, 255, 25);
	delay (200);
	dumbRGBShow(0, 0, 0, 0, 25);

}

//RF Listening to DMX packets loop
void loop(void){
   if( radio.Listen()){
     dumbRGBShow(buffer[0], buffer[1], buffer[2], buffer[3] ,1);
   }
}
 

void dumbRGBShow( int r, int g, int b , int w, int d) {
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
  analogWrite(WHITEPIN, w);
 delay(d);
}
