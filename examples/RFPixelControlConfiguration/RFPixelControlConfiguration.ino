#include <util.h>

#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include "printf.h"
#include <OTAConfig.h>

#include <Arduino.h>

#include <IPixelControl.h>


/*\
*  Created on: June 30 2013
*      Author: Greg Scull
*/

#define RF_DELAY 2000


/***************************  CONFIGURATION SECTION *************************************************/
#define NRF_TYPE MINIMALIST_SHIELD
//#define NRF_TYPE RF1_1_3

//#define NRF_TRANSMITTER_DATARATE RF24_250KBPS
#define NRF_TRANSMITTER_DATARATE RF24_1MBPS
/***************************  CONFIGURATION SECTION *************************************************/

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
	radio.Initalize( radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, NRF_TRANSMITTER_DATARATE, 1);
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
	str[IDX_RF_LISTEN_RATE] = NRF_TRANSMITTER_DATARATE;
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
	str[IDX_RF_LISTEN_RATE] = NRF_TRANSMITTER_DATARATE;
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

