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

#include <IPixelControl.h>
#include <WM2999.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
/**
 
 */

//Uncomment for serial
#define DEBUG 0

//NRF24L01+ Items

//#define outPin 19  //Arduino pin # that Lights are Connected to.  This is actually Pin #28 on the Atmega 328 IC
#define lightCount 20  //Total # of lights on string (usually 50, 48, or 36)


byte gotstr[32];
uint16_t counter=0;
uint8_t led_counter=0;
uint16_t dmx_counter=0;
int pkt_begin=0;
int  pkt_max=0;
int  z=0;
bool readytoupdate=false;



/**************CONFIGURATION SECTION ***************************/
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RFPixelControl radio(9,8);


#define OVER_THE_AIR_CONFIG_ENABLE 0
#define LISTEN_CHANNEL 100

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

byte data[32];

//Arduino setup function.
void setup() {
 printf_begin();
 
  radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	 
	 
	 if(!OVER_THE_AIR_CONFIG_ENABLE)
	 {
		
		 radio.AddLogicalController(0, 1, 464,  0);
	 }
	Serial.begin(57600);
	Serial.write("Initializing reciever\n");



	Serial.write("Initializing Radio\n");
//	 radio.Initalize( radio.RECIEVER pipes, TRANSMIT_CHANNEL, RF24_250KBPS, 50 );
	radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,0);
  radio.printDetails(); 
	Serial.write("Init and Paint LEDS for startup \n");
 //radio.DisplayDiagnosticStartup(&strip) ;
// channels = radio.GetControllerDataBase(0);
}
     



//RF Listening to DMX packets loop
void loop(void){
int packetOffset = 0;
 boolean done= radio.read( &data, 32 );
  if (done){
    packetOffset =  data[30] *30;
    printf("\n\r--%3d:", packetOffset);
      for ( int i =0;i<30;i++){
    
		printf(" 0x%02X", data[i]);
		if ( (i-7) % 8 == 0 )
		{
			printf("\n\r--%3d:", i  + packetOffset);
		}
  
  }

}
}
