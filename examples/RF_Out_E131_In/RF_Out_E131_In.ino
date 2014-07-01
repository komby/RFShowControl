/*
* ArduinoEthernet Shield E1.31 Transmitter
*
* Streaming ACN (E1.31) receiver to RF transmitter for RFPixelControl receivers
* This code can be used to send data to any RFPixelControl Packet format compatible
* Receiving device
*
* This sketch can be used with Unicast or Multicast.  It is recommended to set the ip address and use in unicast mode for best performance.
*
* Created on: June 17, 2013
* Updated: June 25, 2014
* Author: Greg Scull
*/


#include <Dhcp.h>
#include <Dns.h>
#include <EEPROM.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <util.h>

#include "E131Constants.h"
#include "printf.h"
#include "RFPixelControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        MINIMALIST_SHIELD

// UNIVERSE Description: http://learn.komby.com/wiki/58/configuration-settings#UNIVERSE
// Valid Values: 1-255
#define UNIVERSE                        1

// MAC Address Description: http://learn.komby.com/wiki/58/configuration-settings#Mac-Address
static uint8_t mac[] = { 0x5B, 0xD0, 0x00, 0xEA, 0x80, 0x85 };
// IP Address Description: http://learn.komby.com/wiki/58/configuration-settings#IP-Address
static uint8_t ip[] =  { 192, 168, 1, 99 };
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// TRANSMIT_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#TRANSMIT_CHANNEL
// Valid Values: 1-124
#define TRANSMIT_CHANNEL                100

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1

// REFRESH_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#REFRESH_RATE
#define REFRESH_RATE                    0
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
#define RF_WRAPPER                      1
//Include this after all configuration variables are set
#include "RFPixelControlConfig.h"


//END Configuration
//Where in the packet does the dmx start
#define DMX_CHANNEL_DATA_START          126 //DMX Packet Position 0xA8
#define DMX_MAX_CHANNEL                 512


uint8_t buffer1[638]; //buffer to hold incoming packet,
uint8_t buffer2[638]; //buffer to cache good universe packet
uint8_t* buf;
uint8_t* validBuf;


byte str[32];
EthernetUDP Udp;


bool validateRoot(uint8_t buf[]);
bool validateDMP(uint8_t buf[]);
bool validateFraming(uint8_t buf[]);
void transmitDataFromBuffer(uint8_t* pBuffer);

void setup(void)
{

  uint16_t PortNr = 5568;
  Ethernet.begin(mac,ip);
  Udp.begin(PortNr);

  Serial.begin(115200);
  Serial.println(F("\n[E1.31 Arduino Ethernet Transmitter ]\n"));
  printf_begin();

  Serial.print(F("freeMemory()="));
  //Serial.println(freeMemory());
  radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE);
  delayMicroseconds(1500);

  radio.printDetails();
}
int maxprint=0;
int len=0;
int freecount=0;
long duration = millis();
uint16_t numChannelsInPacket;
uint8_t numPackets;
uint16_t universe;
int _rfinterpacketdelay = DATA_RATE == RF24_1MBPS ? 700 : 1500;
boolean status;

void loop(void)
{
  int packetSize = Udp.parsePacket();
  if (packetSize > 0)
  {
    if (validBuf == buffer2)
    {
      buf=buffer1;
    }
    else
      buf=buffer2;

    Udp.read(buf, 638);

    const uint64_t pPipes[2] =
    {
      0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
    };

    bool validateR = validateRoot(buf);
    bool vdmp = validateDMP(buf);
    bool vfr = validateFraming(buf);

    if (validateR && vdmp && vfr)
    {
      duration = millis();

      int channelPos=0;

      numChannelsInPacket = ((buf[E1_31_DMP_PROP_VAL_CNT] <<8) | buf[E1_31_DMP_PROP_VAL_CNT+1]) -1 ;
      numPackets = numChannelsInPacket / 30;
      universe = ((buf[E1_31_FRAMING_UNIVERSE_ID] <<8) | buf[E1_31_FRAMING_UNIVERSE_ID+1]);

        transmitDataFromBuffer(buf);
    }
  }
  else if (REFRESH_RATE && millis() - duration >= REFRESH_RATE)
  {
    //no data received re-send
    transmitDataFromBuffer(validBuf);
    duration = millis();
  }
}

void transmitDataFromBuffer(uint8_t* pBuffer)
{
  int r;

  if (numChannelsInPacket <= DMX_MAX_CHANNEL && numChannelsInPacket >0 && numPackets <= 18 && universe == UNIVERSE)
  {
    validBuf = pBuffer;
    for (r = 0; r< numPackets; r++)
    {
      memcpy(&str[0], &validBuf[r*30 + E1_31_DMP_FIRST], 30);
      str[30] = r;
      radio.write_payload(&str[0], 32);
      delayMicroseconds(_rfinterpacketdelay);
    }
    //final packet if its a partial packet
    int test = numChannelsInPacket - ((r) * 30);

    if (test >0)
    {
      memcpy(&str[0], &validBuf[(r)*30 + E1_31_DMP_FIRST], test);
      str[30] = r;
      radio.write_payload(&str[0], 32);
      delayMicroseconds(_rfinterpacketdelay);
    }
  }
}

bool validateDMP(uint8_t buf[])
{
  if (buf[E1_31_DMP_VECTOR] != E1_31_DMP_VECTOR_VALID
    || buf[E1_31_DMP_ADDRESS_TYPE_DATA_TYPE] != E1_31_DMP_ADDRESS_TYPE_DATA_TYPE_VALID
    || ntos(buf + E1_31_DMP_FIRST_PROP_ADDR) != E1_31_DMP_FIRST_PROP_ADDR_VALID
    || ntos(buf + E1_31_DMP_ADDRESS_INCREMENT) != E1_31_DMP_ADDRESS_INCREMENT_VALID)
  {
    return false;
  }

  return true;
}

bool validateFraming(uint8_t buf[])
{
  if (ntoi(buf + E1_31_FRAMING_VECTOR) != E1_31_FRAMING_VECTOR_VALID)
  {
    return false;
  }

  return true;
}

bool validateRoot(uint8_t buf[])
{
  if (ntos(buf) != RLP_PREAMBLE_SIZE_VALID
    || ntos(buf + RLP_POST_AMBLE_SIZE) != RLP_POST_AMBLE_SIZE_VALID
    || ntoi(buf + IDX_VECTOR_PROTOCOL) != 0x00000004)
  {
    return false;
  }

  return true;
}
