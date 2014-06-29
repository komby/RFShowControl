#ifndef __PACKETS_H__
#define __PACKETS_H__

struct __attribute__ ((packed)) ControllerInitInfo
{
  uint8_t packetType; // 0 for controller init; 1 for logical controller;
  uint32_t controllerID; // controller ID
  uint16_t rfListenChannel;
  uint32_t rfListenRate;
  uint16_t numLogicalControllers;
  uint8_t extra[16];
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

#endif //__PACKETS_H__
