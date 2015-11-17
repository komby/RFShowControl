/*
 * RFShowControl.h
 * This class is built to serve as an extension of the RF24 library for use in Pixel controllers
 * This code was created based on the RF ColorNode code by Joe Johnson
 * it has been refactored such that the RF24 core library is not needed
 * to be modified in order to be used.
 *
 * Created on: Mar, 2013
 * Author: Greg Scull - komby@komby.com
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * License:
 *    Users of this software agree to hold harmless the creators and
 *    contributors of this software.  By using this software you agree that
 *    you are doing so at your own risk, you could kill yourself or someone
 *    else by using this software and/or modifying the factory controller.
 *    By using this software you are assuming all legal responsibility for
 *    the use of the software and any hardware it is used on.
 *
 *    The Commercial Use of this Software is Prohibited.
 */

#ifndef __RFSHOWCONTROL_H__
#define __RFSHOWCONTROL_H__

 
#include "EEPROMUtils.h"
#include "IRFShowControl.h"
#include "OTAConfig.h"
#include "RF24Wrapper.h"

#define RF_CONFIG_STARTUP_WINDOW        10000
#define RF_PACKET_LENGTH                32

#define CHANNELS_PER_PACKET             30  //how many bytes of data are in each packet
#define PACKET_SEQ_IDX                  30  //each packet has a sequence number, what index 0-31 is it?
#define BYTES_PER_PACKET                30  //TODO refactor....


//NRF PINOUTS
#define RF1_1_2                         1
#define RF1_1_3                         2
#define RF1                             2
#define RF1_12V                         2
#define MINIMALIST_SHIELD               3
#define RF1_0_2                         4
#define RF1_12V_0_1                     5
#define WM_2999_NRF                     6
#define KOMBYONE_DUE                    7
#define RFCOLOR_2_4                     8
#define KOMBEE                          9
#define KOMBLINKIN						10
#define RAPTOR12						11
#define MEGA_SHIELD                     12
#define RF1_SERIAL                      13

//PIXEL TYPES
#define GECE                            1
#define WS_2801                         2
#define WS_2811                         3
#define RENARD                          4
#define WM_2999                         5
#define LPD_6803                        6
#define GWTS_EARS                       7
#define LPD_8806                        8
#define SM_16716                        9
#define TM_1809                         10
#define TM_1803                         11
#define UCS_1903                        12
#define DMX                             13
#define STROBE							14
#define WS_2812B                        15
#define NEOPIXEL						16
#define TM_1829							17
#define APA_104						    18
#define LPD_1886						19
#define NONE                            999

//RGB_BOARD_TYPE definitions
#define DUMB_RGB						1
#define FLOODUINO						2

class RFShowControl : public RF24Wrapper
{
public:
  virtual ~RFShowControl(void);

  /*
   * Constructor
   *
   * Creates a new instance of this driver.  Before using, you create an instance
   * and send in the unique pins that this chip is connected to.
   *
   * @param _cepin The pin attached to Chip Enable on the RF module
   * @param _cspin The pin attached to Chip Select
   *
   */
  RFShowControl(uint8_t _cepin, uint8_t _cspin);


  /*
   * Setup the radio as a transmitter or Receiver.
   * The values for this are stored in
   * the TRANSMITTER and RECEIVER values.
   */
  bool Initialize(int role, const uint64_t *pPipes, int pChannel, rf24_datarate_e pDataRate, int pControllerId);

  void SetStartAndEndChannels(void);

  void DisplayDiagnosticStartup(IRFShowControl *string);

  /************************************************************************
   * Listen will be listening for all the data needed for an update
   * When it returns true its time to update all controllers
   ************************************************************************/
  bool Listen(void);

  int SaveConfigurationToEEPROM(void);

  int ReadConfigurationFromEEPROM(void);

  void PrintControllerConfig(ControllerInfo pControllerInfo);

  bool GetControllerDataUpdate(void);

  bool ProcessPacket(byte *dest, byte *src);

  uint8_t *GetControllerDataBase(uint8_t pLogicalControllerNumber);
  /**
  ConfigureReceiverAtStartup
  Returns true if OTAConfig is received.
  */
  bool ConfigureReceiverAtStartup(uint32_t pReceiverId);

  int processConntrollerConfigPacket(uint8_t *pConfigPacket);

  void processLogicalConfigPacket(uint8_t *pLogicalConfigPacket);

  int32_t littleToBigEndianLong(uint8_t *lowByteAddrPtr) ;

  int32_t convert8BitTo32Bit(uint8_t *highByteAddrPtr);

  bool EnableOverTheAirConfiguration(uint8_t enabled);

  int GetNumberOfChannels(int pLogicalController);

  void AddLogicalController(uint8_t pLControllerName, uint32_t pLcontrollerStartChannel, uint32_t pLControllerNumChannels, uint32_t pLControllerBaudRate);

  void PrintControllerConfig(void);

private:


  ControllerInfo *_controllers;
  uint8_t _numControllers;
  uint32_t _controllerId;
  byte packetData[RF_PACKET_LENGTH];
  uint8_t *channelData;
  uint32_t _startChannel;
  uint32_t _endChannel;
  bool _otaConfigEnable;
};

#endif //__RFSHOWCONTROL_H__
