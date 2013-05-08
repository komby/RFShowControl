/*
 * RFPixelControl.h
 * This class is built to serve as an extension of the RF24 library for use in Pixel controllers
 * This code was created based on the RF ColorNode code by Joe Johnson
 * it has been refactored such that the RF24 core library is not needed
 * to be modified in order to be used.
 *
 *  Created on: Mar, 2013
 *      Author: Greg Scull - komby@komby.com
 */

#ifndef RFPIXELCONTROL_H_
#define RFPIXELCONTROL_H_

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <IPixelControl.h>

typedef enum { role_transmitter = 1, role_receiver = 0} role_e;


class RFPixelControl : public RF24 {
public:
	//RFPixelAdapter();
	virtual ~RFPixelControl();

	static const int TRANSMITTER = 1;
	static const int  RECEIVER = 0;


	 /**
	   * Constructor
	   *
	   * Creates a new instance of this driver.  Before using, you create an instance
	   * and send in the unique pins that this chip is connected to.
	   *
	   * @param _cepin The pin attached to Chip Enable on the RF module
	   * @param _cspin The pin attached to Chip Select

	   */
	RFPixelControl(uint8_t _cepin, uint8_t _cspin) ;


	/**
	 * Setup the radio as a transmitter or Receiver.
	 * The values for this are stored in
	 * the TRANSMITTER and RECEIVER values.
	 */
	bool Initalize( int  role, const uint64_t * pPipes, int pChannel );

	
	/**
	 * Check what channel the radio is listeneing on
	 */
	 uint8_t GetChannel(void);
	 
	/**
	  * overriding method from base class
	  *
	  * Based on code suggestion from Joe
	  * overrides the base  SPI.setClockDivider(SPI_CLOCK_DIV4);
	  * to the needed  SPI.setClockDivider(SPI_CLOCK_DIV2);
	  *
	  * @param mode HIGH to take this unit off the SPI bus, LOW to put it on
	 */
	 void  csn(int mode);
	/**
	   * Write the transmit payload
	   *
	   * The size of data written is the fixed payload size, see getPayloadSize()
	   *
	   * @param buf Where to get the data
	   * @param len Number of bytes to be sent
	   * @return Current value of status register
	   */
	  uint8_t write_payload(const void* buf, uint8_t len);
	  
	  void DisplayDiagnosticStartup(IPixelControl * string);
	  uint8_t get_status(void);

private:
	  uint8_t csn_pin; /**< SPI Chip select redefined because private in base ---  its hacky*/
      int _channel;  //channel to transmit on
	  bool dataRateSuccess;
	  bool payloadSizeSetSuccessful;
	  bool channelSetSuccessfully;
	  

};

#endif /* RFPIXELCONTROL_H_ */
