#ifndef __OTA_PACKETS_H__
#define __OTA_PACKETS_H__

struct LogicalInitInfo
{
  uint8_t packetType; // 0 for controller init; 1 for logical controller;
  uint32_t controllerID; // controller ID
  uint8_t  logicalControllerNumber;
  uint32_t startChannel;
  uint32_t numChannels; //num channels is individual channels RGB = 3 channels.
  uint8_t  outputFormat; // DMX or Renard
  uint32_t baudRate;  //if outputting on a dmx or serial device this will be set
  uint8_t extra[13];
};

struct ControllerInitInfo
{
  uint8_t packetType; // 0 for controller init; 1 for logical controller;
  uint32_t controllerID; // controller ID
  uint16_t  rfListenChannel;
  uint32_t rfListenRate;
  uint16_t numLogicalControllers;
  uint8_t extra[16];
};

#endif //__OTA_PACKETS_H__
