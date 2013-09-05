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
RFPixelControl::RFPixelControl(uint8_t _cepin, uint8_t _cspin) :  RF24Wrapper(_cepin, _cspin)
{
	this->_numControllers=0;
	this->_startChannel=0;
	this->_endChannel=0;
	
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
bool RFPixelControl::Initialize( int  pRole, const uint64_t * pPipes, int pChannel, rf24_datarate_e pDataRate, int pNodeId )
{
	bool r = false;
	this->_controllerId = pNodeId;
	this->_channel =pChannel;
	this->dataRateSuccess = false;
	this->payloadSizeSetSuccessful = false;
	//this->_channel = pChannel;
	this->begin();  //initalize RF
	this->setRetries(0,0);  //set # of retries & delay between retries
	this->dataRateSuccess=this->setDataRate( pDataRate );  //set RF data rate
	this->setPayloadSize(32);   //set RF packet size
	
	if ( this->getPayloadSize() == 32 )
	{
		this->payloadSizeSetSuccessful = true;
	}
	
	this->setAutoAck(0);  //Turn off Auto Ack!!!!
	this->setChannel(pChannel); //Change from the default channel...
	
	this->setCRCLength(RF24_CRC_16 );  //Setup CRC

	if ( pRole == TRANSMITTER)
	{
		this->setChannel(pChannel); //Change from the default channel...
		channelSetSuccessfully = false;
		if ( this->GetChannel() == pChannel)
		{
			this->channelSetSuccessfully = true;
		}
		this->openWritingPipe(pPipes[0]);  //Open pipe for Writing
		this->openReadingPipe(1,pPipes[1]);  //Open pipe for Reading...But we aren't reading anything....
		//this->setPALevel(RF24_PA_HIGH);  //Set the power level to high!
		this->setPALevel(RF24_PA_MAX);  //Set the power level to high!
		
		this->write_register(CONFIG, ( this->read_register(CONFIG) | _BV(PWR_UP) ) & ~_BV(PRIM_RX) );  //set up radio for writing!
		this->flush_tx();  //Clear the TX FIFO Buffers
		this->powerUp();  //Fire up the radio
		this->ce(HIGH);  //Turn on transmitter!
		return true;
	}

	else	
	{
		this->channelData = (uint8_t*)calloc(512,sizeof(uint8_t));
		if (this->_otaConfigEnable)
		{
			printf("OTACONFIG-Begin\n");
			//We are setting up a receiver,  Before we can come online we need configuration information
			//Check to see if we have a configuration node online.
			this->setChannel(RF_NODE_CONFIGURATION_CHANNEL); //Change from the default channel...
			
			//Setup Receiver to listen for configuration packets on the Configuration Channel
			this->openWritingPipe(pPipes[1]);  //Open pipe for Writing
			this->openReadingPipe(1,pPipes[0]);  //Open pipe for Reading
			this->startListening();  //Start Listening!
			
			delay(2);
			printf("print config channel details\n");
			printDetails();
			long elapsed = 0;
			long maxTimeout = OTA_CONFIG_WINDOW;
			//&& (millis() - configurationTime )<= RF_CONFIG_STARTUP_WINDOW
			printf("attempting OTA Config \n\t#");

//			for(	unsigned long configurationTime = millis();!this->ConfigureReceiverAtStartup(pNodeId) && elapsed < maxTimeout ;)
			unsigned long configurationTime = millis();
			bool configged=false;
			while (!configged && elapsed < maxTimeout )
			{
				configged = this->ConfigureReceiverAtStartup(pNodeId);
				//delay 2 milliseconds between trying to get configuration data
				//delay(2);
				elapsed = millis() - configurationTime;
				if (!(elapsed % 1000))
				printf("#");
			}
			if (this->_controllers ==NULL || this->_numControllers <= 0)
			{
				//We were not successfull in getting OTA Configuration,
				printf("\nUnable to OTAConfig\n");
				//First try and get configuration from EEPROM
				int eepromVersion = -1;
				if (eeprom_read_int(EEPROM_VERSION_IDX, &eepromVersion))
				{
					printf("EEPROM VERSION IS %d\n",eepromVersion);
					if (eepromVersion == EEPROM_VERSION)
					{
						if( eeprom_read_bytes(EEPROM_CONTROLLER_CONFIG_IDX, this->packetData, EEPROM_PACKET_SIZE))
						{
							int logicalControllerCount = processConntrollerConfigPacket(this->packetData);
							for ( int i=0; i< logicalControllerCount; i++)
							{
								eeprom_read_bytes(EEPROM_BASE_LOGICAL_CONTROLLER_CONFIG_IDX+(i*EEPROM_PACKET_SIZE), this->packetData, EEPROM_PACKET_SIZE);
								processLogicalConfigPacket(packetData);
								
							}
						}
					}
					else
					{
						printf("EEPROM Version Mismatch U need to fix this or else santa is going to give you coal....\n");
						//	return false
					}
				}
				
				
			}
			
				//OTA Successful,  Save to EEPROM,  Reset Listen channel and rate;
				printf("OTAConfig Success\n");
				
				this->stopListening();
				this->setChannel(this->_channel);
				//this->setDataRate(this->_rf_data_rate);
				//this->openWritingPipe(pPipes[1]);  //Open pipe for Writing
				//this->openReadingPipe(1,pPipes[0]);  //Open pipe for Reading
				//this->startListening();  //Start Listening!
				//this->printDetails();
				//SetStartAndEndChannels();
				
			
		}
		
		this->setChannel(this->_channel); //Change from the default channel...
		channelSetSuccessfully = false;
		if ( this->GetChannel() == pChannel)
		this->channelSetSuccessfully = true;
		this->SetStartAndEndChannels();
		
		//setup as a receiver
		this->openWritingPipe(pPipes[1]);  //Open pipe for Writing
		this->openReadingPipe(1,pPipes[0]);  //Open pipe for Reading
		this->startListening();  //Start Listening!
		r=true;
	
}

	
		this->setChannel(this->_channel); //Change from the default channel...
		//channelSetSuccessfully = false;
		
		this->_rf_data_rate = pDataRate;
		
		this->channelSetSuccessfully = true;
		
		//Setup Receiver to listen for configuration packets on the Configuration Channel
		this->openWritingPipe(pPipes[1]);  //Open pipe for Writing
		this->openReadingPipe(1,pPipes[0]);  //Open pipe for Reading
		this->startListening();  //Start Listening!
		
		
	
	printf("after setup\n");
	this->PrintControllerConfig()	  ;
	return r;
	
}

/************************************************************************/
/* AddLogicalController												  */
/************************************************************************/
void RFPixelControl::AddLogicalController(uint8_t pLControllerName, uint32_t pLcontrollerStartChannel, uint32_t pLControllerNumChannels, uint32_t pLControllerBaudRate)
{
//	printf("alc1%d\n", this->_numControllers);
	ControllerInfo* t = this->_controllers;
	this->_controllers= new ControllerInfo[this->_numControllers+1];
	int i=0;
	for (; i< this->_numControllers; i++)
	{
		this->_controllers[i].baudRate = t[i].baudRate;
		this->_controllers[i].numChannels =  t[i].numChannels;
		
		this->_controllers[i].startChannel = t[i].startChannel;
		this->_controllers[i].logicalControllerNumber = t[i].logicalControllerNumber;
		//t[i].customConfig;
		//t[i].reserved;//
	}
	
	this->_numControllers++;

	//printf("Alc %d & %d \n", this->_numControllers, i );
	this->_controllers[i].logicalControllerNumber = i;
	this->_controllers[i].startChannel = pLcontrollerStartChannel;
	this->_controllers[i].numChannels = pLControllerNumChannels;
	this->_controllers[i].baudRate = pLControllerBaudRate;
	//   this->PrintControllerConfig()	  ;
	int num =0;
	this->_startChannel = this->_controllers[0].startChannel;
	
	for ( int i = 0; i < this->_numControllers; i++)
	{
		num += this->_controllers[i].numChannels;
	}
	this->_endChannel = this->_startChannel + num;

}

/************************************************************************/
/* Diagnostic print                                                                     */
/************************************************************************/
void RFPixelControl::PrintControllerConfig(ControllerInfo pControllerInfo)
{
	
	printf("LNumber: \t\t%d\n",pControllerInfo.logicalControllerNumber );
	printf("StartChannel: \t\t\t %lu \n ",pControllerInfo.startChannel);
	printf("NumChannels: \t\t\t %lu \n", pControllerInfo.numChannels);
	printf("BaudRateIfSerial:\t\t\t %lu \n",pControllerInfo.baudRate);

}

/************************************************************************/
/* Diagnostic print     all controllers								  */
/************************************************************************/
void RFPixelControl::PrintControllerConfig(void)
{
	printf("PhysicalController Info: \t\t%d\n", this->_controllerId);
	printf("Logical Controllers : \t\t%d\n", this->_numControllers);
	printf("Listen: \t\t\t%d\n", this->_channel);
	printf("DataStart : \t\t\t % lu \n", this->_startChannel );
	printf("DataEnd:  \t\t\t% lu \n", this->_endChannel);
	printf("NumberOf : \t\t\t%d\n", this->_endChannel - this->_startChannel);
	for (int i=0;i< this->_numControllers;i++)
	{
		PrintControllerConfig(this->_controllers[i]);
	}

}

/************************************************************************/
/* Return the pointer for the base data of the current logical controller
/************************************************************************/
uint8_t* RFPixelControl::GetControllerDataBase( uint8_t pLogicalControllerNumber )
{
	int numChannelsOffset = 0;
	for ( int i = 0; i < this->_numControllers; i++){
		if (this->_controllers[i].logicalControllerNumber == pLogicalControllerNumber){
			printf("GetControllerDataBase1\n");
			PrintControllerConfig();
			if (numChannelsOffset ==0)
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

void RFPixelControl::SetStartAndEndChannels()
{

	//update overall start and end range for all controllers on this device
	this->_startChannel = this->_controllers[0].startChannel;
	//count all the channels on this receiver
	this->_endChannel=0;
	for (int i=0;i<this->_numControllers;i++){
		this->_endChannel += this->_controllers[i].numChannels;
	}
	//add the count of channels to the _startChannel
	this->_endChannel = this->_startChannel + this->_endChannel;
}


int RFPixelControl::processConntrollerConfigPacket(uint8_t* pConfigPacket)
{

	int rfListenRate = (int) pConfigPacket[IDX_RF_LISTEN_RATE];;
	this->_numControllers = 0;//we will let the logical controllers up this number as they get processed.
	int numLogic =  pConfigPacket[IDX_NUMBER_OF_LOGICAL_CONTROLLERS];
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


void  RFPixelControl::processLogicalConfigPacket(uint8_t* pLogicalConfigPacket)
{

	printf("OTA lp %d\n", pLogicalConfigPacket[IDX_LOGICAL_CONTROLLER_NUMBER]);

	uint32_t baudrate = 0;
	switch( pLogicalConfigPacket[IDX_CONFIG_PACKET_TYPE])
	{
		case LOGICALCONTROLLER_LED:
		case LOGICALCONTROLLER_CUSTOM:


		break;
		case LOGICALCONTROLLER_SERIAL:
			
			baudrate = convert8BitTo32Bit(&pLogicalConfigPacket[IDX_LOGICAL_CONTROLLER_CLOCK_OR_BAUD]);
			printf("BaudRate is set to %lu\n", baudrate);
			break;
			//memcpy(&ciTemp->reserved, &this->packetData[IDX_LOGICAL_CONTROLLER_RESERVED], RESERVED_BYTES_LEN);
			//memcpy(&ciTemp->customConfig, &this->packetData[IDX_CUSTOM_CONTROLLER_CONFIG_SPACE], CUSTOM_CONFIG_INFO_LEN);

	}
	this->AddLogicalController(pLogicalConfigPacket[IDX_CONTROLLER_ID], convert8BitTo32Bit(pLogicalConfigPacket+IDX_LOGICAL_CONTROLLER_START_CHANNEL),convert8BitTo32Bit(pLogicalConfigPacket + IDX_LOGICAL_CONTROLLER_NUM_CHANNELS), baudrate);
}
	
	int32_t RFPixelControl::convert8BitTo32Bit(uint8_t *  highByteAddrPtr)
	{
		return (uint32_t)highByteAddrPtr[0] << 24|
		(uint32_t)highByteAddrPtr[1] << 16|
		(uint32_t)highByteAddrPtr[2] << 8|
		(uint32_t)highByteAddrPtr[3];	
	}
	

/************************************************************************/
/* OTA Configuration handler
/************************************************************************/
bool RFPixelControl::ConfigureReceiverAtStartup(uint32_t pReceiverId) {

									//Get the current time,  This will break out of configuration if longer than RF_CONFIG_STARTUP_WINDOW
	ControllerInfo * ciTemp;
	bool returnValue = false;
	int rfListenChannel = 0;
	int rfListenRate = 0;
	int numberOfLogicalControllers = 0;

	if (this->available())
	{
		
	
		// Fetch the payload, and see if this was the last one.
		this->read( &this->packetData, 32 );
		
	//	printf("got packet ... what its?");
		//The First Configuration Packet contains the number of logical controllers for a given controller
		if(this->packetData[IDX_CONFIG_PACKET_TYPE] == CONTROLLERINFOINIT && ( convert8BitTo32Bit(this->packetData+IDX_CONTROLLER_ID)  == pReceiverId))
		{
#ifdef DEBUG_PRINT
			printf("received Config Packet for controller\n");
			for ( int i =0;i<32 ;i++)
			{
				printf(" 0x%02X", this->packetData[i]);
				if ( (i-7) % 8 == 0 )
				{
					printf("\n\r--%3d:", i  );
				}
			}
#endif
			int numLogic = processConntrollerConfigPacket(this->packetData);
					//processConntrollerConfigPacket();
			//save the controller config packet into the eeprom.
			eeprom_erase_all();
			delay(1000);
			uint32_t version = EEPROM_VERSION;
			eeprom_write_bytes(EEPROM_VERSION_IDX, (byte * ) &version, 4 );
			eeprom_write_bytes(EEPROM_CONTROLLER_CONFIG_IDX, (byte * )this->packetData, EEPROM_PACKET_SIZE );
			int lControllerCount = 0;

			//get each logical controller
			for(int i = 0; i < numLogic;)
			{
				//printf("innumlogicloop %d \n", i);
				while(!this->available());
				delay(4);
				for(bool found=false; this->read( &this->packetData, 32 ) && !found;)
				{

#ifdef DEBUG_PRINT
					printf("received OTA logical\n");
					for ( int i =0;i<32 ;i++)
					{
							printf(" 0x%02X", this->packetData[i]);
							if ( (i-7) % 8 == 0 )
							{
    							printf("\n\r--%3d:", i  );
							}
					}
#endif
	//delay(6);
	//make sure we are on a logical controller packet
	//that we have the next logical controller ID
	//and that its for this physical receiver
	//delay(1000);
					if (this->packetData[IDX_CONFIG_PACKET_TYPE] != CONTROLLERINFOINIT &&(convert8BitTo32Bit(this->packetData+IDX_CONTROLLER_ID) == pReceiverId )&& this->packetData[IDX_LOGICAL_CONTROLLER_NUMBER]==i)
					{


						processLogicalConfigPacket(this->packetData);
#ifdef DEBUG_PRINT
						printf("configured Logical Packet\n");
#endif
						found=true;
						//if the logical packet was processed we can save it for later.
						eeprom_write_bytes(EEPROM_BASE_LOGICAL_CONTROLLER_CONFIG_IDX+i, (byte * )this->packetData, EEPROM_PACKET_SIZE);


						//only increment the loop counter when we have the packet
						i++;

					}
					if (i==this->_numControllers)	
					{
						returnValue = true;
					}
			}
		}	
		printf("eeprom:");
		eeprom_serial_dump_table();
		//delay(1000);
		}
	}
	return returnValue;
}



void RFPixelControl::DisplayDiagnosticStartup(IPixelControl * string)
{
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





bool RFPixelControl::Listen(void)
{
	// See if there is any data in the RF buffer
	if ( this->available() ){
		for (bool done = false;!done;){
			// Fetch the payload, and see if this was the last one.
			done = this->read( &this->packetData, 32 );
			//when process packet returns true we got the last channel we are listening to and its time to output....
			if (ProcessPacket( this->channelData, this->packetData))
			{
			//return true to the sketch and let it handle updates
				return true;
			}
		}
	}
	return false;
}

	/**
	*  processPacket - handles copying the needed channels from the radio data into the Renard data.
	*        This method takes a different approach for getting the data.  Originally I was looping through
	*        all the channels and I got bored.  So I wrote this convoluted code hoping to make things faster by using memcpy
	*        I haven't done performance timing so who knows?  It passed my tests :)
	*
	*  return - true if the last channel was found and its time for an update, otherwise false.
	*/
	bool  RFPixelControl::ProcessPacket(byte*  dest, byte* p)
	{
		int startChannel = this->_startChannel -1;
		int finalChannel = this->_endChannel- 1;

		//  Way too many variables here need to refactor...
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

		//first assume we will be copying the whole packet.
		int channelCopyStartIdx = 0;
		int channelCopyEndIdx = CHANNELS_PER_PACKET - 1;
		int numberOfValidChannelsInPacket = channelCopyEndIdx;
							
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
		//start channel was after the range of channels in this packet.  skip this packet.
		return retVal;
		}
		}
		else
		{
		calcStartChannel =  packetStartChann;
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
		}

		}
		else
		{
		//final channel is before the end of this packet  (packetEndChannel >= finalChann )
		if (finalChannel >= packetStartChann)
		{
		//Because final is less than the packet end
		//and >= start we know that the final channel is in this packet
		calcEndChannel =  this->_endChannel ;
		calcEndDestIdx = finalChannel - startChannel ;
		calcEndSourceIdx = finalChannel - packetStartChann;

		//because this is the last packet we care about,  return true
		retVal=true;

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
		memcpy(&dest[calcStartDestIdx], &p[calcStartSourceIdx], numChannelsInPacket);

	}
	return retVal;
}



bool RFPixelControl::EnableOverTheAirConfiguration(uint8_t enabled)
{
	if(enabled>0)
		this->_otaConfigEnable = true;
	else
		this->_otaConfigEnable = false;
}

int RFPixelControl::GetNumberOfChannels(int pLogicalController)
{
	return this->_controllers[pLogicalController].numChannels;
}


