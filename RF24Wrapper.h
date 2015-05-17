/*
 * Author: Greg Scull, komby@komby.com
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

#ifndef __RF24WRAPPER_H__
#define __RF24WRAPPER_H__

#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

class RF24Wrapper :  public RF24
{
public:
  virtual ~RF24Wrapper(void);

  static const int TRANSMITTER = 1;
  static const int RECEIVER = 0;


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
  RF24Wrapper(uint8_t _cepin, uint8_t _cspin);


  /**
  * Expose your friend methods from RF24
  * Read single byte from a register
  *
  * @param reg Which register. Use constants from nRF24L01.h
  * @return Current value of register @p reg
  */
//  uint8_t read_register(uint8_t reg);

  /*
   * Check what channel the radio is listening on
   */
  uint8_t GetChannel(void);

  bool ChangeTransmitChannel(int transmitChannel);

  ///*
  // * overriding method from base class
  // *
  // * Based on code suggestion from Joe
  // * overrides the base SPI.setClockDivider(SPI_CLOCK_DIV4);
  // * to the needed SPI.setClockDivider(SPI_CLOCK_DIV2);
  // *
  // * @param mode HIGH to take this unit off the SPI bus, LOW to put it on
  // */
  //void csn(int mode);

  //void ce(bool);

  /**
  * Write a single byte to a register
  *
  * @param reg Which register. Use constants from nRF24L01.h
  * @param value The new value to write
  * @return Current value of status register
  */
//  uint8_t write_register(uint8_t reg, uint8_t value);


  /*
   * Write the transmit payload
   *
   * The size of data written is the fixed payload size, see getPayloadSize()
   *
   * @param buf Where to get the data
   * @param len Number of bytes to be sent
   * @return Current value of status register
   */  
//  uint8_t write_payload(const void* buf, uint8_t len, const uint8_t writeType);
//  uint8_t write_payload(const void* buf, uint8_t len);

//  uint8_t get_status(void);

  


  bool Initialize(int pRole, const uint64_t *pPipes, int pChannel, rf24_datarate_e pDataRate, int logical = 0);

protected:
 // uint8_t csn_pin; // SPI Chip select redefined because private in base --- its hacky
  int _channel; //channel to transmit on
  rf24_datarate_e _rf_data_rate; //Data Rate for the RF.

  bool dataRateSuccess;
  bool payloadSizeSetSuccessful;
  bool channelSetSuccessfully;
  const uint64_t *pipes;
};

#endif //__RF24WRAPPER_H__
