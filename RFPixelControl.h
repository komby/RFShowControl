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
#include <OTAConfig.h>

#define RF_CONFIG_STARTUP_WINDOW 10000
#define RF_PACKET_LENGTH  32

#define CHANNELS_PER_PACKET 30    //how many bytes of data are in each packet
#define PACKET_SEQ_IDX 30         //each packet has a sequence number,  what index 0-31 is it?
#define BYTES_PER_PACKET 30       //TODO refactor....


//NRF PINOUTS
#define RF1_1_2				1
#define RF1_1_3				2
#define MINIMALIST_SHIELD	3
#define RF1_0_2				4
#define RF1_12V_0_1			5
#define WM_2999_NRF			6
#define KOMBYONE_DUE        7

//PIXEL TYPES
#define GECE				1
#define WS_2801				2
#define WS_2811				3
#define RENARD				4
#define WM_2999				5
#define LPD_6803			6

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
	bool Initialize( int  role, const uint64_t * pPipes, int pChannel, rf24_datarate_e pDataRate, int pControllerId);

	void SetStartAndEndChannels();

	
	/**
	* Check what channel the radio is listening on
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
	
	/************************************************************************
	* Listen will be listening for all the data needed for an update
	* When it returns true its time to update all controllers
	************************************************************************/
	bool Listen(void);
	
	int SaveConfigurationToEEPROM();
	
	
	int ReadConfigurationFromEEPROM();
	
	void PrintControllerConfig(ControllerInfo pControllerInfo);
	
	bool GetControllerDataUpdate( void );
	bool ProcessPacket(byte*  dest, byte* src);
	uint8_t* GetControllerDataBase( uint8_t pLogicalControllerNumber );
	/**
	ConfigureReceiverAtStartup
	Returns true if OTAConfig is received.
	*/
	bool ConfigureReceiverAtStartup(uint32_t pReceiverId);
	
	bool EnableOverTheAirConfiguration(uint8_t enabled);
	
	int GetNumberOfChannels(int pLogicalController);
	void AddLogicalController(int pLControllerName, int pLcontrollerStartChannel, int pLControllerNumChannels, int pLControllerBaudRate);
	
	void PrintControllerConfig(void);

	private:
	uint8_t csn_pin; /**< SPI Chip select redefined because private in base ---  its hacky*/
	int _channel;  //channel to transmit on
	rf24_datarate_e _rf_data_rate; //Data Rate for the RF.
	
	bool dataRateSuccess;
	bool payloadSizeSetSuccessful;
	bool channelSetSuccessfully;
	ControllerInfo * _controllers;
	uint8_t _numControllers;
	uint32_t _controllerId;
	byte packetData[RF_PACKET_LENGTH];
	uint8_t * channelData;
	int _startChannel;
	int _endChannel;
	bool _otaConfigEnable;
	
	

	};

	#endif /* RFPIXELCONTROL_H_ */
