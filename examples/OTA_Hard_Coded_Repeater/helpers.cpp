#include "utility/util.h"
#include "helpers.h"
#include "OTAConfig.h"
#include "RF24Wrapper.h"

static int count;
extern RF24Wrapper radio;

void sendInitPacket(uint32_t id, uint16_t listen_channel, uint32_t rate, uint16_t logical_controllers)
{
	count = 0;
	ControllerInitInfo init = {0};

	init.packetType = CONTROLLERINFOINIT;
	init.controllerID = htonl(id);
	init.rfListenChannel = listen_channel;
	init.rfListenRate = rate;
	init.numLogicalControllers = logical_controllers;

	sendPacket((char *)&init);
}

void send_Renard_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels, uint32_t baud)
{
	LogicalInitInfo packet = {0};

	packet.packetType = LOGICALCONTROLLER_SERIAL;
	packet.controllerID = htonl(id);
	packet.logicalControllerNumber = count++;
	packet.startChannel = htonl(start_channel);
	packet.numChannels = htonl(number_of_channels);
	packet.outputFormat = RENARD_OUTPUT;
	packet.baudRate = htonl(baud);

	sendPacket((char*)&packet);
}

void send_DMX_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels)
{
	LogicalInitInfo packet = {0};

	packet.packetType = LOGICALCONTROLLER_SERIAL;
	packet.controllerID = htonl(id);
	packet.logicalControllerNumber = count++;
	packet.startChannel = htonl(start_channel);
	packet.numChannels = htonl(number_of_channels);
	packet.outputFormat = DMX_OUTPUT;
	packet.baudRate = htonl(250000);

	sendPacket((char*)&packet);
}

void send_LED_OTA_Packet(uint32_t id, uint32_t start_channel, uint32_t number_of_channels)
{
	LogicalInitInfo packet = {0};

	packet.packetType = LOGICALCONTROLLER_LED;
	packet.controllerID = htonl(id);
	packet.logicalControllerNumber = count++;
	packet.startChannel = htonl(start_channel);
	packet.numChannels = htonl(number_of_channels);

	sendPacket((char*)&packet);
}

void print_buffer(uint8_t *buffer, uint8_t length)
{
	Serial.print("0x");
	for (int i=0; i<length; i++) {
		if (buffer[i]<0x10) {Serial.print("0");}
		Serial.print(buffer[i],HEX);
		Serial.print(" ");
	}
	Serial.println("");
}

void sendPacket(char *packet)
{
  Serial.println("Sending packet...");
  print_buffer((uint8_t *)packet, 32);

  radio.writeFast((uint8_t *)packet, 32, false);
  delay(100);
}

