/*
 * 
 *
 *  Created on: Mar  2013
 *      Author: Greg Scull, komby@komby.com
 *
 *   Print packet content and optionally packet data snooping on transmitter channels
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

//Uncomment for serial output of all data
#define FULL_PRINT_DEBUG 0


//NRF24L01+ Items



byte gotstr[32];




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
	Serial.write("Initializing DEBUG listener\n");

	radio.Initalize( radio.RECEIVER, pipes, LISTEN_CHANNEL,RF24_1MBPS ,0);
  radio.printDetails(); 
	Serial.write("Init and Paint LEDS for startup \n");

}
     



//RF Listening to DMX packets loop
void loop(void){
  int packetOffset = 0;
  if ( radio.available() )
  {
     radio.read( &data, 32 );
    
        packetOffset =  data[30] *30;
        printf("\n\r--%3d:",  data[30] );
        for ( int i =0;i<30 && FULL_PRINT_DEBUG;i++)
        {
          printf(" 0x%02X", data[i]);
  	  if ( (i-7) % 8 == 0 )
	  {
	    printf("\n\r--%3d:", i  + packetOffset);
	  }
        }
     
  }
}
