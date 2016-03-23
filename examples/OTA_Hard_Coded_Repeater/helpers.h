#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <stdint.h>

struct __attribute__ ((packed)) ControllerInitInfo
{
	uint8_t packetType; // 0 for controller init; 1 for logical controller;
	uint32_t controllerID; // controller ID
	uint16_t rfListenChannel;
	uint32_t rfListenRate;
	uint16_t numLogicalControllers;
	uint8_t extra[19];
};

struct __attribute__ ((packed)) LogicalInitInfo
{
	uint8_t packetType; // LOGICALCONTROLLER_LED=2, LOGICALCONTROLLER_SERIAL=3, LOGICALCONTROLLER_CUSTOM=9;
	uint32_t controllerID; // controller ID
	uint8_t logicalControllerNumber; //start with 0 -> n-1
	uint32_t startChannel;
	uint32_t numChannels; //num channels is individual channels RGB = 3 channels.
	uint8_t outputFormat; //OPTIONAL DMX=2 or Renard=1
	uint32_t baudRate; //OPTIONAL, used with LOGICALCONTROLLER_SERIAL & LOGICALCONTROLLER_CUSTOM if outputting on a dmx or serial device
	uint8_t reserved[3]; //3 bytes future use
	uint8_t customconfig[10];//10 bytes of optional configuration data.
};

void sendPacket(char *packet);

void sendInitPacket(uint32_t id, uint16_t listen_channel, uint32_t rate, uint16_t logical_controllers);
void send_Renard_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels, uint32_t baud);
void send_DMX_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels);
void send_LED_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels);

#endif // _HELPERS_H_
