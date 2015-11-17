/*
 * RFShowControl.cpp
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

#include "RFShowControl.h"


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
RFShowControl::RFShowControl(uint8_t _cepin, uint8_t _cspin) : RF24Wrapper(_cepin, _cspin), _otaConfigEnable(false)
{
  this->_numControllers = 0;
  this->_startChannel = 0;
  this->_endChannel = 0;
}

RFShowControl::~RFShowControl(void)
{
  // TODO Auto-generated destructor stub
}

/*
 * Initalize Method
 * Setup the radio as a transmitter or Receiver.
 * The values for this are stored in
 * the TRANSMITTER and RECEIVER values.
 *
 * @param role - the role (TRANSMITTER or RECEIVER)
 * @param pPipes - the addressses to listen on
 */
bool RFShowControl::Initialize(int pRole, const uint64_t *pPipes, int pChannel, rf24_datarate_e pDataRate, int pNodeId)
{
  bool r = false;
  this->_controllerId = pNodeId;
  this->_channel = pChannel;
  this->dataRateSuccess = false;
  this->payloadSizeSetSuccessful = false;
  this->begin(); //initalize RF
  this->setRetries(0,0); //set # of retries & delay between retries
  this->dataRateSuccess = this->setDataRate(pDataRate); //set RF data rate
  this->setPayloadSize(32); //set RF packet size

  if (this->getPayloadSize() == 32)
  {
    this->payloadSizeSetSuccessful = true;
  }

  this->setAutoAck(0); //Turn off Auto Ack!!!!
  this->setChannel(pChannel); //Change from the default channel...

  this->setCRCLength(RF24_CRC_16); //Setup CRC

  if (pRole == TRANSMITTER)
  {
    this->setChannel(pChannel); //Change from the default channel...\-
    channelSetSuccessfully = false;
    if (this->GetChannel() == pChannel)
    {
      this->channelSetSuccessfully = true;
    }
    this->openWritingPipe(pPipes[0]); //Open pipe for Writing
    this->openReadingPipe(1,pPipes[1]); //Open pipe for Reading...But we aren't reading anything....
    this->setPALevel(RF24_PA_MAX); //Set the power level to high!
	//TODO Find a method in the new RF24 library to replace teh below method.
   // this->write_register(CONFIG, (this->read_register(CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX)); //set up radio for writing!
    this->flush_tx(); //Clear the TX FIFO Buffers
    this->powerUp(); //Fire up the radio
   // this->ce(HIGH); //Turn on transmitter!
    return this->channelSetSuccessfully;
  }

  else
  {
    this->channelData = (uint8_t*)calloc(1024,sizeof(uint8_t));
    if (this->_otaConfigEnable)
    {
      printf_P(PSTR("OTACONFIG-Begin\n"));
      //We are setting up a receiver, Before we can come online we need configuration information
      //Check to see if we have a configuration node online.
      this->setChannel(RF_NODE_CONFIGURATION_CHANNEL); //Change from the default channel...
	  
	  //When doing OTA Configuration we will modify the data rate to 250kbps and reset the data rate after.
	  this->dataRateSuccess = this->setDataRate(RF24_250KBPS); //set RF data rate

      //Setup Receiver to listen for configuration packets on the Configuration Channel
      this->openWritingPipe(pPipes[1]); //Open pipe for Writing
      this->openReadingPipe(1,pPipes[0]); //Open pipe for Reading
      this->startListening(); //Start Listening!

      delay(2);

      printDetails();
      long elapsed = 0;
      long maxTimeout = OTA_CONFIG_WINDOW;
      printf_P(PSTR("attempting OTA Config \n\t#"));

      unsigned long configurationTime = millis();
      bool configged = false;
      while (!configged && elapsed < maxTimeout)
      {
        configged = this->ConfigureReceiverAtStartup(pNodeId);
        elapsed = millis() - configurationTime;
        if (!(elapsed % 1000))
        printf("#");
      }
      if (this->_controllers == NULL || this->_numControllers <= 0)
      {
        //We were not successful in getting OTA Configuration,
        printf_P(PSTR("\nUnable to OTAConfig\n"));
        //First try and get configuration from EEPROM
        int eepromVersion = -1;
        if (eeprom_read_int(EEPROM_VERSION_IDX, &eepromVersion))
        {
          printf_P(PSTR("EEPROM VERSION IS %d\n"),eepromVersion);
          if (eepromVersion == EEPROM_VERSION)
          {
            if(eeprom_read_bytes(EEPROM_CONTROLLER_CONFIG_IDX, this->packetData, EEPROM_PACKET_SIZE))
            {
              int logicalControllerCount = processConntrollerConfigPacket(this->packetData);
              for (int i = 0; i< logicalControllerCount; i++)
              {
                eeprom_read_bytes(EEPROM_BASE_LOGICAL_CONTROLLER_CONFIG_IDX+(i*EEPROM_PACKET_SIZE), this->packetData, EEPROM_PACKET_SIZE);
                processLogicalConfigPacket(packetData);

              }
            }
          }
          else
          {
            printf_P(PSTR("EEPROM Version Mismatch U need to fix this or else santa is going to give you coal....\n"));
            return false;
          }
        }

      }

      //OTA Successful, Save to EEPROM, Reset Listen channel and rate;
      printf_P(PSTR("OTAConfig Success\n"));
      this->stopListening();
      this->setChannel(this->_channel);
      this->setDataRate(this->_rf_data_rate);
    }

    this->setChannel(this->_channel); //Change from the default channel...
    channelSetSuccessfully = false;
    if (this->GetChannel() == pChannel)
    this->channelSetSuccessfully = true;
    this->SetStartAndEndChannels();

    //setup as a receiver
    this->openWritingPipe(pPipes[1]); //Open pipe for Writing
    this->openReadingPipe(1,pPipes[0]); //Open pipe for Reading
    this->startListening(); //Start Listening!
    return this->channelSetSuccessfully;
  }


  this->setChannel(this->_channel); //Change from the default channel...

  this->_rf_data_rate = pDataRate;
  this->setDataRate(this->_rf_data_rate);
  this->channelSetSuccessfully = true;
  //Setup Receiver to listen for configuration packets on the Configuration Channel
  this->openWritingPipe(pPipes[1]); //Open pipe for Writing
  this->openReadingPipe(1,pPipes[0]); //Open pipe for Reading
  this->startListening(); //Start Listening!
  this->PrintControllerConfig();
  return r;
}

/************************************************************************/
/* AddLogicalController                          */
/************************************************************************/
void RFShowControl::AddLogicalController(uint8_t pLControllerName, uint32_t pLcontrollerStartChannel, uint32_t pLControllerNumChannels, uint32_t pLControllerBaudRate)
{
  ControllerInfo* t = this->_controllers;
  this->_controllers = new ControllerInfo[this->_numControllers+1];
  int i=0;
  for (; i< this->_numControllers; i++)
  {
    this->_controllers[i].baudRate = t[i].baudRate;
    this->_controllers[i].numChannels = t[i].numChannels;

    this->_controllers[i].startChannel = t[i].startChannel;
    this->_controllers[i].logicalControllerNumber = t[i].logicalControllerNumber;
    //t[i].customConfig;
    //t[i].reserved;//
  }

  this->_numControllers++;
  this->_controllers[i].logicalControllerNumber = i;
  this->_controllers[i].startChannel = pLcontrollerStartChannel;
  this->_controllers[i].numChannels = pLControllerNumChannels;
  this->_controllers[i].baudRate = pLControllerBaudRate;
  int num = 0;
  this->_startChannel = this->_controllers[0].startChannel;

  for (int i = 0; i < this->_numControllers; i++)
  {
    num += this->_controllers[i].numChannels;
  }
  this->_endChannel = this->_startChannel + num;
}

/************************************************************************/
/* Diagnostic print                            */
/************************************************************************/
void RFShowControl::PrintControllerConfig(ControllerInfo pControllerInfo)
{
  printf_P(PSTR("LNumber:\t%d\n"),pControllerInfo.logicalControllerNumber);
  printf_P(PSTR("StartCh:\t%lu\n"),pControllerInfo.startChannel);
  printf_P(PSTR("NumCh:\t%lu\n"), pControllerInfo.numChannels);
  printf_P(PSTR("Baud:\t%lu\n"),pControllerInfo.baudRate);

}

/************************************************************************/
/* Diagnostic print all controllers                    */
/************************************************************************/
void RFShowControl::PrintControllerConfig(void)
{
   printf_P(PSTR("Cntl Info:\t%d\n"), this->_controllerId);
   printf_P(PSTR("LCntrl:\t%d\n"), this->_numControllers);
   printf_P(PSTR("Listen:\t%d\n"), this->_channel);
   printf_P(PSTR("Start:\t%lu\n"), this->_startChannel);
   printf_P(PSTR("End:\t% lu\n"), this->_endChannel);
   printf_P(PSTR("Num:\t%d\n"), this->_endChannel - this->_startChannel);
  for (int i = 0; i < this->_numControllers; i++)
  {
    PrintControllerConfig(this->_controllers[i]);
  }
}

/**************************************************************************/
/* Return the pointer for the base data of the current logical controller */
/**************************************************************************/
uint8_t* RFShowControl::GetControllerDataBase(uint8_t pLogicalControllerNumber)
{
  int numChannelsOffset = 0;
  for (int i = 0; i < this->_numControllers; i++){
    if (this->_controllers[i].logicalControllerNumber == pLogicalControllerNumber){
      printf_P(PSTR("GetControllerDataBase1\n"));
      PrintControllerConfig();
	  
      if (numChannelsOffset == 0)
      {
        return this->channelData;
      }
      else
      {
        return &this->channelData[numChannelsOffset -1];
      }
    }
    else {
    numChannelsOffset += this->_controllers[i].numChannels;
    }
  }

  PrintControllerConfig();
}

void RFShowControl::SetStartAndEndChannels(void)
{

  //update overall start and end range for all controllers on this device
  this->_startChannel = this->_controllers[0].startChannel;
  //count all the channels on this receiver
  this->_endChannel = 0;
  for (int i = 0; i < this->_numControllers; i++){
    this->_endChannel += this->_controllers[i].numChannels;
  }
  //add the count of channels to the _startChannel
  this->_endChannel = this->_startChannel + this->_endChannel;
}


int RFShowControl::processConntrollerConfigPacket(uint8_t *pConfigPacket)
{

  int rfListenRate = (int) pConfigPacket[IDX_RF_LISTEN_RATE];;
  this->_numControllers = 0;//we will let the logical controllers up this number as they get processed.
  int numLogic = pConfigPacket[IDX_NUMBER_OF_LOGICAL_CONTROLLERS];
  this->_channel = pConfigPacket[IDX_RF_LISTEN_CHANNEL];;

  switch (rfListenRate)
  {
    case RF24_250KBPS:
      this->_rf_data_rate = RF24_250KBPS;
    break;
    case RF24_1MBPS:
      this->_rf_data_rate = RF24_1MBPS;
    break;
    case RF24_2MBPS:
      this->_rf_data_rate = RF24_2MBPS;
    break;
  }

  return numLogic;
}


void RFShowControl::processLogicalConfigPacket(uint8_t *pLogicalConfigPacket)
{
  printf_P(PSTR("OTA Logical Packet "));
  printf("%d\n", pLogicalConfigPacket[IDX_LOGICAL_CONTROLLER_NUMBER]);

  uint32_t baudrate = 0;
  switch(pLogicalConfigPacket[IDX_CONFIG_PACKET_TYPE])
  {
    case LOGICALCONTROLLER_LED:
    case LOGICALCONTROLLER_CUSTOM:


    break;
    case LOGICALCONTROLLER_SERIAL:

      baudrate = convert8BitTo32Bit(&pLogicalConfigPacket[IDX_LOGICAL_CONTROLLER_CLOCK_OR_BAUD]);
      printf_P(PSTR("BaudRate is set to "));
      printf("%lu\n", baudrate);
      break;

  }
  this->AddLogicalController(pLogicalConfigPacket[IDX_CONTROLLER_ID], convert8BitTo32Bit(pLogicalConfigPacket+IDX_LOGICAL_CONTROLLER_START_CHANNEL),convert8BitTo32Bit(pLogicalConfigPacket + IDX_LOGICAL_CONTROLLER_NUM_CHANNELS), baudrate);
}

  int32_t RFShowControl::convert8BitTo32Bit(uint8_t *highByteAddrPtr)
  {
    return (uint32_t)highByteAddrPtr[0] << 24|
    (uint32_t)highByteAddrPtr[1] << 16|
    (uint32_t)highByteAddrPtr[2] << 8|
    (uint32_t)highByteAddrPtr[3];
  }


/************************************************************************/
/* OTA Configuration handler                      */
/************************************************************************/
bool RFShowControl::ConfigureReceiverAtStartup(uint32_t pReceiverId) {

  //Get the current time, This will break out of configuration if longer than RF_CONFIG_STARTUP_WINDOW
  ControllerInfo *ciTemp;
  bool returnValue = false;
  int rfListenChannel = 0;
  int rfListenRate = 0;
  int numberOfLogicalControllers = 0;

  if (this->available())
  {
    // Fetch the payload, and see if this was the last one.
    this->read(&this->packetData, 32);

    //The First Configuration Packet contains the number of logical controllers for a given controller
    if(this->packetData[IDX_CONFIG_PACKET_TYPE] == CONTROLLERINFOINIT && (convert8BitTo32Bit(this->packetData+IDX_CONTROLLER_ID) == pReceiverId))
    {
#ifdef DEBUG_PRINT
      printf_P(PSTR("received Config Packet for controller\n"));
      for (int i = 0; i < 32; i++)
      {
        printf(" 0x%02X", this->packetData[i]);
        if ((i-7) % 8 == 0)
        {
          printf("\n\r--%3d:", i);
        }
      }
#endif
      int numLogic = processConntrollerConfigPacket(this->packetData);
      //save the controller config packet into the eeprom.
      eeprom_erase_all(0xFF);
      delay(1000);
      uint32_t version = EEPROM_VERSION;
      eeprom_write_bytes(EEPROM_VERSION_IDX, (byte *) &version, 4);
      eeprom_write_bytes(EEPROM_CONTROLLER_CONFIG_IDX, (byte *)this->packetData, EEPROM_PACKET_SIZE);
      int lControllerCount = 0;

      //get each logical controller
      for(int i = 0; i < numLogic;)
      {
        while(!this->available());
        delay(4);
        for(bool found = false;  !found ;)
        {
			this->read(&this->packetData, 32);
#ifdef DEBUG_PRINT
          printf_P(PSTR("received OTA logical\n"));
          for (int i = 0; i < 32; i++)
          {
              printf(" 0x%02X", this->packetData[i]);
              if ((i-7) % 8 == 0)
              {
                printf("\n\r--%3d:", i);
              }
          }
#endif
  //delay(6);
  //make sure we are on a logical controller packet
  //that we have the next logical controller ID
  //and that its for this physical receiver
  //delay(1000);
          if (this->packetData[IDX_CONFIG_PACKET_TYPE] != CONTROLLERINFOINIT &&(convert8BitTo32Bit(this->packetData+IDX_CONTROLLER_ID) == pReceiverId)&& this->packetData[IDX_LOGICAL_CONTROLLER_NUMBER] == i)
          {


            processLogicalConfigPacket(this->packetData);
#ifdef DEBUG_PRINT
            printf_P(PSTR("configured Logical Packet\n"));
#endif
            found = true;
            //if the logical packet was processed we can save it for later.
            eeprom_write_bytes(EEPROM_BASE_LOGICAL_CONTROLLER_CONFIG_IDX+i, (byte *)this->packetData, EEPROM_PACKET_SIZE);


            //only increment the loop counter when we have the packet
            i++;

          }
          if (i == this->_numControllers)
          {
            returnValue = true;
          }
      }
    }
    printf_P(PSTR("eeprom:"));
    eeprom_serial_dump_table();
    //delay(1000);
    }
  }
  return returnValue;
}

void RFShowControl::DisplayDiagnosticStartup(IRFShowControl *string)
{
  if (payloadSizeSetSuccessful && dataRateSuccess && channelSetSuccessfully)
  {
    for (int i = 0 ; i < string->GetElementCount(); i++)
    {
      string->SetElementColor(i, 0, 255, 0);
    }
  }
  else
  {
    for (int i = 0 ; i < string->GetElementCount(); i++)
    {
      string->SetElementColor(i, 255, 0, 0);
    }
  }

  string->Paint();
  delay(10000);
}

bool RFShowControl::Listen(void)
{
  // See if there is any data in the RF buffer
  if (this->available())
  {
      // Fetch the payload, and see if this was the last one.
	    this->read(&this->packetData, 32);
      
      //when process packet returns true we got the last channel we are listening to and its time to output....
      if (ProcessPacket(this->channelData, this->packetData))
      {
        //return true to the sketch and let it handle updates
        return true;
      }
  }
  return false;
}

/*
 * processPacket - handles copying the needed channels from the radio data into the Renard data.
 *  This method takes a different approach for getting the data.  Originally I was looping through
 *  all the channels and I got bored.  So I wrote this convoluted code hoping to make things faster by using memcpy
 *  I haven't done performance timing so who knows?  It passed my tests :)
 *
 * return - true if the last channel was found and its time for an update, otherwise false.
 */
bool RFShowControl::ProcessPacket(byte *dest, byte *p)
{
  int startChannel = this->_startChannel -1;
  int finalChannel = this->_endChannel- 1;

  // Way too many variables here need to refactor...
  bool retVal = false;
  int packetSequence = p[PACKET_SEQ_IDX];
  int packetStartChann = packetSequence * CHANNELS_PER_PACKET;
  int packetEndChannel = packetStartChann + CHANNELS_PER_PACKET;

  int calcStartChannel = -1;
  int calcStartDestIdx = -1;
  int calcStartSourceIdx = -1;


  int calcEndChannel = -1;
  int calcEndDestIdx = -1;
  int calcEndSourceIdx = -1;

  //First we need to know what is the first valid channel in this packet
  if (startChannel >= packetStartChann)
  {
    //It could be in the packet is it?
    if (startChannel <= packetEndChannel)
    {
      //set calculated start to the start channel
      calcStartChannel = startChannel;
      //set the idx of the dest array to the start channel
      calcStartDestIdx = 0 ;
      //offset the idx for source by the channel count factor
      calcStartSourceIdx = startChannel - packetStartChann;
      //start is after the packet start and before or equal to the end channel.
    }
    else
    {
      //start channel was after the range of channels in this packet. skip this packet.
      return retVal;
    }
  }
  else
  {
    calcStartChannel = packetStartChann;
    calcStartDestIdx = packetStartChann - startChannel;
    calcStartSourceIdx = 0;
    //we started before this packet
    //end channel interrogation will handle if we need
    //anything from this packet.
  }
  //now interrogate the end channel
  //check if the end is in this packet
  if (packetEndChannel <= finalChannel)
  {
    //since final is greater than the end of the packet, check start
    //if start is set we need all channels from this packet.
    if (calcStartChannel >= 0)
    {
      calcEndChannel = packetEndChannel;
      calcEndDestIdx = packetEndChannel - (startChannel);
      calcEndSourceIdx = packetEndChannel - packetStartChann;

      // MM 2014/06/29:
      // If we end on a packet boundary and no further packets are sent before
      // looping, we previously never set retVal to true in this scenario and
      // would never print our received data to the pixels.
      if ( packetEndChannel == finalChannel )
      {
        retVal = true;
      }
    }
  }
  else
  {
    //final channel is before the end of this packet (packetEndChannel >= finalChann)
    if (finalChannel >= packetStartChann)
    {
      //Because final is less than the packet end
      //and >= start we know that the final channel is in this packet
      calcEndChannel = this->_endChannel;
      calcEndDestIdx = finalChannel - startChannel;
      calcEndSourceIdx = finalChannel - packetStartChann;

      //because this is the last packet we care about, return true
      retVal = true;
    }
    else
    {
      //Final happened before this packet//continue.
      return retVal;
    }
  }

  //now we have calculated positions
  if (calcStartChannel >= 0 && calcEndChannel >= 0)
  {
    //how many channels are we getting from this packet?
    int numChannelsInPacket = calcEndChannel - calcStartChannel;

    //Use memcpy to copy the bytes from the radio packet into the data array.
   // printf("seq %d : Start %d ,  End %d , fc %d psc %d |%d | %d |  numch %d \n", packetSequence, calcStartDestIdx,  calcStartSourceIdx, finalChannel, packetStartChann , calcStartChannel, calcEndChannel, numChannelsInPacket);
    memcpy(&dest[calcStartDestIdx], &p[calcStartSourceIdx], numChannelsInPacket);

  }
  return retVal;
}



bool RFShowControl::EnableOverTheAirConfiguration(uint8_t enabled)
{
  if(enabled>0)
    this->_otaConfigEnable = true;
  else
    this->_otaConfigEnable = false;
}

int RFShowControl::GetNumberOfChannels(int pLogicalController)
{
  return this->_controllers[pLogicalController].numChannels;
}
