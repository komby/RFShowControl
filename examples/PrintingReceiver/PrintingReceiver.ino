/*
*
*
*  Created on: July  2013
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
#include <EEPROM.h>
/**************CONFIGURATION SECTION ***************************/

//Uncomment for serial output of all data
#define FULL_PRINT_DEBUG 0

//NRF24L01+ Items
//What board are you using to connect your nRF24L01+?
//Valid Values:  MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF, RFCOLOR2_4
#define NRF_TYPE			RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//What RF Channel do you want to listen on?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	

/**************END CONFIGURATION SECTION ***************************/


//THESE OPTIONS WERE REMOVED FROM THE CONFIGURATION SECTION
// YOU DO NOT NEED TO MODIFY THEM
#define OVER_THE_AIR_CONFIG_ENABLE 0
#define RECEIVER_UNIQUE_ID 0
//END

//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>

byte data[32];

void setup() {
	printf_begin();
	
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	
	
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		//not used hardcoded for compatibility purposes
		radio.AddLogicalController(0, 1, 10,  0);
	}
	Serial.begin(57600);
	Serial.write("Initializing DEBUG listener\n");

	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
	radio.printDetails();
	Serial.write("Init and Paint LEDS for startup \n");

}




//RF Listening to DMX packets loop
void loop(void){
	int packetOffset = 0;
	if ( radio.available() )
	{
		radio.read( &data, 32 );
		
		packetOffset =  data[30];
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