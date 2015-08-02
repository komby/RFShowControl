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

#include "RF24Wrapper.h"

/*
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
RF24Wrapper::RF24Wrapper(uint8_t _cepin, uint8_t _cspin): RF24(_cepin, _cspin)
{
	pinMode(_cspin, OUTPUT);
}

RF24Wrapper::~RF24Wrapper(void)
{
  // TODO Auto-generated destructor stub
}

//helper to check the channel we tried to set;
uint8_t RF24Wrapper::GetChannel(void)
{
	return RF24::getChannel();
}


/*
 * Initalize Method
 * Setup the radio as a transmitter or Receiver.
 * The values for this are stored in
 * the TRANSMITTER and RECEIVER values.
 *
 * @param role - the role (TRANSMITTER or RECEIVER)
 *
 * @param pPipes - the addressses to listen on
 */
bool RF24Wrapper::Initialize(int pRole, const uint64_t *pPipes, int pChannel, rf24_datarate_e pDataRate, int logical)
{
  bool r = false;

  this->_channel =pChannel;
  this->dataRateSuccess = false;
  this->payloadSizeSetSuccessful = false;
  this->begin(); //initalize RF
  this->setRetries(0,0); //set # of retries & delay between retries
  this->dataRateSuccess=this->setDataRate(pDataRate); //set RF data rate
  this->setPayloadSize(32);  //set RF packet size

  if (this->getPayloadSize() == 32)
  {
    this->payloadSizeSetSuccessful = true;
  }

  this->setAutoAck(0); //Turn off Auto Ack!!!!
  this->setChannel(pChannel); //Change from the default channel...

  this->setCRCLength(RF24_CRC_16); //Setup CRC

  if (pRole == TRANSMITTER)
  {
    this->setChannel(pChannel); //Change from the default channel...
  /*  channelSetSuccessfully = false;
    if (this->GetChannel() == pChannel)
    {
      this->channelSetSuccessfully = true;
    }*/
	this->channelSetSuccessfully = true;
    this->openWritingPipe(pPipes[0]); //Open pipe for Writing
    this->openReadingPipe(1,pPipes[1]); //Open pipe for Reading...But we aren't reading anything....
    this->setPALevel(RF24_PA_MAX); //Set the power level to high!
	//TODO Find a method in the new RF24 library to replace teh below method.
   // this->write_register(CONFIG, (this->read_register(CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX)); //set up radio for writing!
    this->flush_tx(); //Clear the TX FIFO Buffers
    this->powerUp(); //Fire up the radio
   // this->ce(HIGH); //Turn on transmitter!
    return true;
  }
  else
  {
    //receiver setup

    this->setChannel(this->_channel); //Change from the default channel...
    channelSetSuccessfully = false;
    if (this->GetChannel() == pChannel)
    this->channelSetSuccessfully = true;

    this->pipes=pPipes;
    //setup as a receiver
    this->openWritingPipe(pPipes[1]); //Open pipe for Writing
    this->openReadingPipe(1,pPipes[0]); //Open pipe for Reading
    this->startListening(); //Start Listening!
    r=true;

  }

  return r;
}

bool RF24Wrapper::ChangeTransmitChannel(int pChannel)
{
	this->powerDown();
    this->setChannel(pChannel); //Change from the default channel...

    this->powerUp(); //Turn on transmitter!
  return true;
}

///*
// * Override base class method
// * This method was created so that we dont
// * have to directly modify the rf24 library.
// */
//void RF24Wrapper::csn(int mode)
//{
//  // Minimum ideal SPI bus speed is 2x data rate
//  // If we assume 2Mbs data rate and 16Mhz clock, a
//  // divider of 4 is the minimum we want.
//  // CLK:BUS 8Mhz:2Mhz, 16Mhz:4Mhz, or 20Mhz:5Mhz
//  #ifdef ARDUINO
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE0);
//  SPI.setClockDivider(SPI_CLOCK_DIV2);
//  #endif
//  digitalWrite(csn_pin,mode);
//}

/*
// * Implemented overriding method from the base RF library.
// * This just exposes the otherwise protected method.
// */
//uint8_t RF24Wrapper::write_payload(const void *buf, uint8_t len)
//{
////	//TODO Find out what this writeType is all about?????
// return RF24Wrapper::write_payload(buf, len, 1);
//}

////uint8_t RF24Wrapper::write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
//////fuint8_t RF24Wrapper::write_payload(const void *buf, uint8_t len, const uint8_t writeType)
////{
////
////
////	//TODO Find out what this writeType is all about?????
////	return RF24::write_payload(buf, data_len, writeType);
////}
////uint8_t RF24Wrapper::get_status(void)
////{
////  return RF24::get_status();
////}
//
//uint8_t RF24Wrapper::read_register(uint8_t reg)
//{
//	return RF24::read_register(reg); 
//}
//
//uint8_t RF24Wrapper::write_register(uint8_t reg, uint8_t value)
//{
//	return RF24::write_register(reg, value);
//}
//void RF24Wrapper::ce(bool value)
//{
//	RF24::ce(value);
//}
//
