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
#include "WM2999PixelControl.h"
#include <IPixelControl.h>
#include <WM2999.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
/**
 * The WM2999 receiver is only designed to control 1 string of lights at a time.
 *
 * As the receiver is listening to a Transmitter capable of transmitting 512 channels
 * We need to make sure that we are listening to the right channels in the universe
 *
 * DMX_START_CHANNEL -   This is the first channel that will be used for this string
 * 					      if this is the first string in the universe it should be set to (0 or 1)//TODO Determine 0 or 1
 * DMX_NUM_CHANNELS - 	 This is the total number of channels needed for this string. For RGB (3) x number of pixels = #
 * DMX_LED_CHANNELS 20 - This defines the # of LED Channels.  This could be the same as lightCount, but if grouping is used, this will be less.
 */
#define DMX_START_CHANNEL 0

#define DMX_LED_CHANNELS 20  //This defines the # of LED Channels.  This could be the same as lightCount, but if grouping is used, this will be less.

#define DMX_NUM_CHANNELS 60 //Number of DMX channels to read...usually dmx_led_channels/3


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




// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RFPixelControl radio(9,10);


//Setup the lights on arduino pin A0 
WM2999PixelControl strip =  WM2999PixelControl(A0);


// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };



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
	strip.Start();
	strip.Paint();


	Serial.write("Initializing Radio\n");
	radio.Initalize( radio.RECEIVER, pipes,100 );
  radio.printDetails(); 
	Serial.write("Init and Paint LEDS for startup \n");
	strip.ColorWipe(strip.Color(255, 0, 0), 100);
	strip.Paint();
delay (2000);
 radio.DisplayDiagnosticStartup(&strip) ;
delay (9000);
}
     



//RF Listening to DMX packets loop
void loop(void){

	// See if there is any data in the RF buffer
	if ( radio.available() ){

		for (bool done = false;!done;){

			// Fetch the payload, and see if this was the last one.
			done = radio.read( &gotstr, 32 );

			//Update the led_counter value from the packet address in position 30
			pkt_begin=gotstr[30]*30;
			dmx_counter=pkt_begin;
			pkt_max=pkt_begin+29;

			/* z is the packet byte counter
			 * dmx data is in packet bytes 0 to 29.
			 * Packet address is in byte 30.
			 * Packet size could be reduced to 30...//TODO possibly reduce packet size to 30
			 */
			z=0;


			//Make sure the numbers make sense
			if (pkt_max>=DMX_START_CHANNEL && pkt_begin<DMX_START_CHANNEL+DMX_NUM_CHANNELS) {

				/* This loop is checking to make sure that the packet being looked at
				 * is a channel which is in the subset of channels from the DMX Universe
				 * that this pixel string is in.   If the packet is outside that range
				 * it is skipped.
				 */
				while (dmx_counter<DMX_START_CHANNEL){
					dmx_counter++;
					z=z+1;
				}
				while ((dmx_counter>=DMX_START_CHANNEL) && (dmx_counter<DMX_START_CHANNEL+DMX_NUM_CHANNELS) && (z<30)  && (dmx_counter<508))
				{


					//TODO re-implement the code (removed)  which joe had checking to see if the pixel data had changed before telling it to update.
					//     in this early release of code I have removed this functionality.
					led_counter=(dmx_counter-DMX_START_CHANNEL)/3;

					strip.SetPixelColor(led_counter, gotstr[z], gotstr[z+1], gotstr[z+2]);

					dmx_counter=dmx_counter+3;
					if ((dmx_counter>=DMX_START_CHANNEL+DMX_NUM_CHANNELS) || (dmx_counter>=508)){
						readytoupdate=true;
					}

					//Move the index counter past the 3 LEDs of the current pixel.
					z=z+3;
				}

				//
				//If we have received a full set of data, Update all the LED's that have changed value since the last time
				//
				if (readytoupdate){
					strip.Paint();
					readytoupdate=false;
				}
			}
		}
	}
}

