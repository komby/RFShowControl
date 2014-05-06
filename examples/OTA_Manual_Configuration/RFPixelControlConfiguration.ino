#include <util.h>

#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include "printf.h"
#include <OTAConfig.h>
#include <Arduino.h>

#include <IPixelControl.h>


/*
*  Created on: June 30 2013
*      Author: Greg Scull
*/

#define RF_DELAY 2000


/***************************  CONFIGURATION SECTION *************************************************/
//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
//Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
#define NRF_TYPE  RF1_1_3

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

/***************************  END CONFIGURATION SECTION *************************************************/
#include <RFPixelControlConfig.h>

//change the mac (0xEF, 0xE0, 0xE1, ... 0xE9)
byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change this IP to one on your network
//IPAddress ip(192, 168, 2, 76);


char packetBuffer[700]; //buffer to hold incoming packet,
byte str[32];


void setup(){

	Serial.begin(57600);
	
	printf_begin();
	//radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL );
	radio.Initialize( radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, DATA_RATE, 1);
	delayMicroseconds(5000);

	radio.printDetails();
}
int maxprint=0;
int len=0;
int freecount=0;
long duration = millis();
long durationAfter = 0;
long checkduration = 0;
boolean status;
void loop () {
	
	//FirstPacket
	//ConfigurationDataForReceiver 1
	//
	//COntroller 1  ID 50
	int logicalSequence = 0;
	str[IDX_CONTROLLER_ID]  = 50;
	//The First Configuration Packet contains the number of logical controllers for a given controller
	str[IDX_CONFIG_PACKET_TYPE] = CONTROLLERINFOINIT;
	str[IDX_RF_LISTEN_CHANNEL] = 100;
	str[IDX_RF_LISTEN_RATE] = DATA_RATE;
	str[IDX_NUMBER_OF_LOGICAL_CONTROLLERS] = 1;
	printf("WritingPayloadFor init packet\n");
	radio.write_payload( &str[0], 32 );
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01) {
		status = radio.get_status();
	}
	
	
	str[IDX_CONFIG_PACKET_TYPE] =LOGICALCONTROLLER_LED ;
	str[IDX_CONTROLLER_ID] = 50;
	str[IDX_LOGICAL_CONTROLLER_NUMBER] = logicalSequence;
	str[IDX_LOGICAL_CONTROLLER_NUM_CHANNELS] = 3;
	str[IDX_LOGICAL_CONTROLLER_START_CHANNEL] = 1;
	printf("WritingPayloadFor config packet\n");
	
	radio.write_payload( &str[0], 32 );
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01) {
		status = radio.get_status();
	}
	//FirstPacket
	//ConfigurationDataForReceiver 1
	//
	//COntroller 1  ID 33
	str[IDX_CONTROLLER_ID]  = 33;
	logicalSequence = 0;
	//The First Configuration Packet contains the number of logical controllers for a given controller
	str[IDX_CONFIG_PACKET_TYPE] = CONTROLLERINFOINIT;
	str[IDX_RF_LISTEN_CHANNEL] = 100;
	str[IDX_RF_LISTEN_RATE] = DATA_RATE;
	str[IDX_NUMBER_OF_LOGICAL_CONTROLLERS] = 1;
	printf("WritingPayloadFor init packet\n");
	radio.write_payload( &str[0], 32 );
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01) {
		status = radio.get_status();
	}
	
	
	str[IDX_CONFIG_PACKET_TYPE] = LOGICALCONTROLLER_LED;
	str[IDX_CONTROLLER_ID] = 33;
	str[IDX_LOGICAL_CONTROLLER_NUMBER] = logicalSequence;
	str[IDX_LOGICAL_CONTROLLER_NUM_CHANNELS] = 150;
	str[IDX_LOGICAL_CONTROLLER_START_CHANNEL] = 4;


	printf("WritingPayloadFor config packet\n");
	
	radio.write_payload( &str[0], 32 );
	delayMicroseconds(5000);
	status = radio.get_status();
	while (status & 0x01) {
		status = radio.get_status();
	}
	
	
	
}

