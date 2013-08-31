//#include <EEPROM.h>
//#include <controller.h>
//#include <FastSPI_LED2.h>
//#include <controller.h>
//#include <fastpin.h>
//#include <fastspi.h>
//#include <clockless.h>
//#include <lib8tion.h>
//#include <hsv2rgb.h>
//#include <chipsets.h>
//#include <dmx.h>

#ifndef __OTA_CONFIG_H
#define __OTA_CONFIG_H
#define OTA_CONFIG_WINDOW 50000
#define RF_NODE_CONFIGURATION_CHANNEL 125	
#define EEPROM_VERSION 1 
#define EEPROM_PACKET_SIZE 32
#define EEPROM_VERSION_IDX 0
#define EEPROM_CONTROLLER_CONFIG_IDX 4
#define EEPROM_BASE_LOGICAL_CONTROLLER_CONFIG_IDX EEPROM_CONTROLLER_CONFIG_IDX + EEPROM_PACKET_SIZE
enum ControlPacketType {CONTROLLERINFOINIT=1, LOGICALCONTROLLER_LED=2, LOGICALCONTROLLER_SERIAL=3, LOGICALCONTROLLER_CUSTOM=9};
enum ControllerOutputProtocol { RENARD_OUTPUT=1, DMX_OUTPUT=2};

//to define the 10 byte array storing the custom controller configuration information.
#define CUSTOM_CONFIG_INFO_LEN 10
#define RESERVED_BYTES_LEN 3

//////////////////////////////////////////////////////////////////////////
//This is the structure of the initial packet set to a configurable node
//it has the receiver ID and the number of logical controllers'
//it also has the RFChannel information the receiver will listen on 
//after configuration is complete
//0
//One Byte Control Cod
#define IDX_CONFIG_PACKET_TYPE					0
//1,2,3,4
#define IDX_CONTROLLER_ID						1
//5,6
//RF Listen Channel is a 16Bit Number
#define IDX_RF_LISTEN_CHANNEL					5
//7,8,9,10
//RFListenSpeed - this is the speed the nRF24L01 will listen as (32bit)
#define IDX_RF_LISTEN_RATE						7
//11
#define IDX_NUMBER_OF_LOGICAL_CONTROLLERS    11


//////////////////////////////////////////////////////////////////////////
// These define the Packet Structure of a configuration packet sent from
// the configuration node to a receiver.
// A receiver may receive multiple of these packets.
// Still planning on using 32 bit packets for simplicity.
//
//0
//One Byte Control Code
//#define IDX_CONFIG_PACKET_TYPE					0
//1,2,3,4
//ControllerID is a 32Bit number 
//#define IDX_CONTROLLER_ID						1
//Controller Number is a 8Bit number ( these are logical devices which will run on an individual controller)
//5
#define IDX_LOGICAL_CONTROLLER_NUMBER			5
//6-9 4
//Start Channel is the start channel for a Logical controller it is a 32Bit number
#define IDX_LOGICAL_CONTROLLER_START_CHANNEL			6
//10-13
//NumberOfChannels is the number of channels the Logical Controller will listen to  (32 Bit)
#define IDX_LOGICAL_CONTROLLER_NUM_CHANNELS		10
//14
//OutputFormat - optional
#define IDX_LOGICAL_CONTROLLER_OUTPUT_FORMAT 14
//15-19
//For Renard or DMX devices this will be the baud Rate of the Serial input or output, For pixel types it will be their MHZ if needed (32Bit)
#define IDX_LOGICAL_CONTROLLER_CLOCK_OR_BAUD	15
#define IDX_LOGICAL_CONTROLLER_RESERVED			19

#define IDX_CUSTOM_CONTROLLER_CONFIG_SPACE 22

//DUMB RGB LOGICAL CONTROLLER ALTERNATE PACKET FORMAT

//#define IDX_LOGICAL_CONTROLLER_RGB_RED_PIN  18
//#define IDX_LOGICAL_CONTROLLER_RGB_GREEN_PIN  19
//#define IDX_LOGICAL_CONTROLLER_RGB_BLUE_PIN  20


struct ControllerInfo {
	uint8_t  logicalControllerNumber;
	uint32_t startChannel;
	uint32_t numChannels; //num channels is individual channels RGB = 3 channels.
	uint32_t baudRate;  //if outputting on a dmx or serial device this will be set	
	uint8_t reserved[RESERVED_BYTES_LEN] ;
	uint8_t customConfig[CUSTOM_CONFIG_INFO_LEN];
	
	};
	
#endif;