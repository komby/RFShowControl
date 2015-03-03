/*
 * ArduinoEthernet Shield E1.31 Transmitter
 *
 *    Input: E1.31
 *    Output: nRF
 *
 * Author: Greg Scull
 * Created on: June 17, 2013
 * Updated: June 25, 2014
 * Updated: October 13, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 * 		MAC address in default sketch wouldn't work with certain "smart" switches.
 * 		Jon Chuchla gave useful information regarding mac addresses here:
 * 		http://diychristmas.org/vb1/showthread.php?2760-Can-t-ping-komby-sandwich&p=31396&viewfull=1#post31396
 * Updated: February 19th, 2015 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *		Feedback on chosen default MAC address from DMCole proved that certain network
 *		equipment isn't happy with the 0x12, 0x34 ... type MAC address and will not
 *		allow network traffic through as expected.  Using an address that fits the
 *		format described by Jon, but recommended by Arduino seems to work on both the
 *		older network equipment in use by DMCole as well as Materdaddy's equipment the
 *		October 2014 changes were tested on.
 *
 * Description:
 *     Streaming ACN (E1.31) receiver to RF transmitter for RFShowControl receivers
 *     This code can be used to send data to any RFShowControl Packet format compatible
 *     Receiving device
 *
 * This sketch can be used with Unicast or Multicast.  It is recommended to set the
 * ip address and use in unicast mode for best performance.
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
#include "RFShowControl.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        MINIMALIST_SHIELD

// UNIVERSE Description: http://learn.komby.com/wiki/58/configuration-settings#UNIVERSE
// Valid Values: 1-255
#define UNIVERSE                        1

// MAC Address Description: http://learn.komby.com/wiki/58/configuration-settings#Mac-Address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IP Address Description: http://learn.komby.com/wiki/58/configuration-settings#IP-Address
static uint8_t ip[] =  { 192, 168, 1, 99 };
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// TRANSMIT_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#TRANSMIT_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define TRANSMIT_CHANNEL                10

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
#include "RFShowControlConfig.h"


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

void(* resetFunc) (void) = 0;//declare reset function at address 0

bool validateRoot(uint8_t buf[]);
bool validateDMP(uint8_t buf[]);
bool validateFraming(uint8_t buf[]);
void transmitDataFromBuffer(uint8_t* pBuffer);

void setup(void)
{

  uint16_t PortNr = 5568;
  Ethernet.begin(mac, ip);
  Udp.begin(PortNr);

  Serial.begin(115200);
  Serial.println(F("\n[E1.31 Arduino Ethernet Transmitter ]\n"));
  printf_begin();

  if(radio.Initialize(radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE)){
     Serial.println("Radio Is UP");
  }else{
     resetFunc(); //If nrf failes reset 
  }
  
 
  //Convert the output from the ethernet lib to same format as ip array   
  uint32_t value = Ethernet.localIP();
  uint8_t ipFromEthernet[] = {value, value >> 8, value >> 16, value  >> 24};
 //check of the hardware ip matches what shoudl have been set.
  if((ipFromEthernet[0] == ip[0]) 
  && (ipFromEthernet[1] == ip[1]) 
  && (ipFromEthernet[2] == ip[2]) 
  && (ipFromEthernet[3] == ip[3])){
    Serial.println("Ethernet Is UP");
  }else{
	resetFunc(); //If nrf failes reset 
  }
  
  
  delayMicroseconds(1500);

  radio.printDetails();

  Serial.print("DMX start channel= ");
  Serial.println(DMX_CHANNEL_DATA_START);
  Serial.print("Komby channel    = ");
  Serial.println(TRANSMIT_CHANNEL);
  Serial.print("IP Address       = ");
  Serial.println(Ethernet.localIP());
  Serial.print("MAC              = ");
  if( mac[0] < 0x10){ Serial.print("0");} Serial.print(mac[0],HEX);
  Serial.print(":");
  if( mac[1] < 0x10){ Serial.print("0");} Serial.print(mac[1],HEX);
  Serial.print(":");
  if( mac[2] < 0x10){ Serial.print("0");} Serial.print(mac[2],HEX);
  Serial.print(":");
  if( mac[3] < 0x10){ Serial.print("0");} Serial.print(mac[3],HEX);
  Serial.print(":");
  if( mac[4] < 0x10){ Serial.print("0");} Serial.print(mac[4],HEX);
  Serial.print(":");
  if( mac[5] < 0x10){ Serial.print("0");} Serial.println(mac[5],HEX);

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
