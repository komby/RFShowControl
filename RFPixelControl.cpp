/*
 * RFPixelControl.cpp
 * This class is built to serve as an extension of the RF24 library for use in Pixel controllers
 * This code was created based on the RF ColorNode code by Joe Johnson
 * it has been refactored such that the RF24 core library is not needed
 * to be modified in order to be used.
 *
 *  Created on: Mar, 2013
 *      Author: Greg Scull - komby@komby.com
 */

#include "RFPixelControl.h"

/**
 * Constructor
 *
 * Creates a new instance of this driver.  Before using, you create an instance
 * and send in the unique pins that this chip is connected to.
 *
 * This constructor calls the related constructor on the RF24 base class.
 *
 * @param _cepin The pin attached to Chip Enable on the RF module
 * @param _cspin The pin attached to Chip Select
 */
RFPixelControl::RFPixelControl(uint8_t _cepin, uint8_t _cspin) :  RF24(_cepin, _cspin), csn_pin( _cspin ) {
}

RFPixelControl::~RFPixelControl() {
	// TODO Auto-generated destructor stub
}

/**
 *  Initalize Method
 * Setup the radio as a transmitter or Receiver.
 * The values for this are stored in
 * the TRANSMITTER and RECEIVER values.
 * @param role - the role (TRANSMITTER or RECEIVER)
 * @param pPipes - the addressses to listen on
 *
 */
 bool RFPixelControl::Initalize( int  pRole, const uint64_t * pPipes, int pChannel ){
	bool r = false;
	this->dataRateSuccess = false;
	this->payloadSizeSetSuccessful = false;
	 this->begin();  //initalize RF
	 this->setRetries(0,0);  //set # of retries & delay between retries
	 this->dataRateSuccess=this->setDataRate( RF24_250KBPS );  //set RF data rate
	 this->setPayloadSize(32);   //set RF packet size
	 
	 if ( this->getPayloadSize() == 32 ){
	    this->payloadSizeSetSuccessful = true;
	 }
	 
	 this->setAutoAck(0);  //Turn off Auto Ack!!!!
	 this->setChannel(pChannel); //Change from the default channel...
	 channelSetSuccessfully = false;
	 if ( this->GetChannel() == pChannel)
		this->channelSetSuccessfully = true;
	 this->setCRCLength(RF24_CRC_16 );  //Setup CRC

	 if ( pRole == TRANSMITTER){
		 this->openWritingPipe(pPipes[0]);  //Open pipe for Writing
		 this->openReadingPipe(1,pPipes[1]);  //Open pipe for Reading...But we aren't reading anything....
		 this->setPALevel(RF24_PA_HIGH);  //Set the power level to high!
		 this->write_register(CONFIG, ( this->read_register(CONFIG) | _BV(PWR_UP) ) & ~_BV(PRIM_RX) );  //set up radio for writing!
		 this->flush_tx();  //Clear the TX FIFO Buffers
		 this->powerUp();  //Fire up the radio
		 this->ce(HIGH);  //Turn on transmitter!
		 r=true;
	 }
	 else {
		 //setup as a receiver
		 this->openWritingPipe(pPipes[1]);  //Open pipe for Writing
		 this->openReadingPipe(1,pPipes[0]);  //Open pipe for Reading
		 this->startListening();  //Start Listening!
		 r=true;
	 }
	 return r;

 }

 void RFPixelControl::DisplayDiagnosticStartup(IPixelControl * string) {
	   
	     if ( payloadSizeSetSuccessful && dataRateSuccess && channelSetSuccessfully ) {
			for ( int i = 0 ; i < string->GetPixelCount(); i++){
				string->SetPixelColor(i, 0, 255, 0);

			}
		 }
		 
		 else {
		 
		 for ( int i = 0 ; i < string->GetPixelCount(); i++){
				string->SetPixelColor(i, 255, 0, 0);

			}
		 
		 }
		 
		 
		 				string->Paint();
						delay(10000);
		
		
	  }
 
 //helper to check the channel we tried to set;
 uint8_t RFPixelControl::GetChannel(void){
 
  //write_register(RF_CH,min(channel,max_channel));
  return read_register(RF_CH);
 
 }
 
 
 /**
  * Override base class method
  * This method was created so that we dont
  * have to directly modify the rf24 library.
  */
 void RFPixelControl::csn(int mode)
 {

   // Minimum ideal SPI bus speed is 2x data rate
   // If we assume 2Mbs data rate and 16Mhz clock, a
   // divider of 4 is the minimum we want.
   // CLK:BUS 8Mhz:2Mhz, 16Mhz:4Mhz, or 20Mhz:5Mhz
 #ifdef ARDUINO
   SPI.setBitOrder(MSBFIRST);
   SPI.setDataMode(SPI_MODE0);
   SPI.setClockDivider(SPI_CLOCK_DIV2);
 #endif
   digitalWrite(csn_pin,mode);
 }


 /**
  * Implemented overriding method from the base RF library.
  * This just exposes the otherwise protected method.
  */
 uint8_t RFPixelControl::write_payload(const void* buf, uint8_t len) {
	 	 return RF24::write_payload(buf, len);
 }

uint8_t RFPixelControl::get_status(void){
   return RF24::get_status();
}