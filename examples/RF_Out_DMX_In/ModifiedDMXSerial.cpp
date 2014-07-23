// - - - - -
// ModifiedDMXSerial - A hardware supported interface to DMX.
// ModifiedDMXSerial.cpp: Library implementation file
//
// This Modified DMX library was created based on the work done by Matthais Hertel:
// For more information about the original work see:
//
// Copyright (c) 2011 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
//
// Changes were made to the DMXSerial library which required the ISR that is used to handle
// both the DMX and the RFShowControl data for effciiencey reasons.  As such this work
// is included in the RFShowControl library for use as a DMX-Wireless bridge.
//
// ModifiedDMXSerial created  2014 - Author: Greg Scull, http://www.komby.com
//

#include <Arduino.h>
#include <avr/interrupt.h>

#include "ModifiedDMXSerial.h"

// ----- Debugging -----

// to debug on an oscilloscope, enable this

//#undef SCOPEDEBUG
//#define SCOPEDEBUG 1
#ifdef SCOPEDEBUG
#define DmxTriggerPin 9  // low spike at beginning of start byte
#define DmxISRPin 3  // low during interrupt service routines
#endif

#define DMX_NUM_CHANNELS 512
// ----- Constants -----

// Define port & bit values for Hardware Serial Port.
// These definitions are done for not hardcoding port 0 in this library.
// If you want to use this library with another hardware port on a more advanced chip,
// you can change the 0 (zero) to 1, 2, 3... in these constant definitions.

#define UCSRnA UCSR0A  // Control and Status Register A

#define UCSRnB UCSR0B  // USART Control and Status Register B

#define RXCIEn RXCIE0  // Enable Receive Complete Interrupt
#define TXCIEn TXCIE0  // Enable Transmission Complete Interrupt
#define UDRIEn UDRIE0  // Enable Data Register Empty Interrupt
#define RXENn  RXEN0   // Enable Receiving
#define TXENn  TXEN0   // Enable Sending

#define UCSRnC UCSR0C  // Control and Status Register C
#define USBSn  USBS0   // Stop bit select 0=1bit, 1=2bits
#define UCSZn0 UCSZ00  // Character size 00=5, 01=6, 10=7, 11=8 bits
#define UPMn0  UPM00   // Parity setting 00=N, 10=E, 11=O

#define UBRRnH UBRR0H  // USART Baud Rate Register High
#define UBRRnL UBRR0L  // USART Baud Rate Register Low

#define UDRn   UDR0    // USART Data Register
#define UDREn  UDRE0   // USART Data Ready
#define FEn    FE0     // Frame Error

// formats for serial transmission
#define SERIAL_8N1  ((0<<USBSn) | (0<<UPMn0) | (3<<UCSZn0))
#define SERIAL_8N2  ((1<<USBSn) | (0<<UPMn0) | (3<<UCSZn0))
#define SERIAL_8E1  ((0<<USBSn) | (2<<UPMn0) | (3<<UCSZn0))
#define SERIAL_8E2  ((1<<USBSn) | (2<<UPMn0) | (3<<UCSZn0))

// the break timing is 10 bits (start + 8 data + parity) of this speed
// the mark-after-break is 1 bit of this speed plus approx 6 usec
// 100000 bit/sec is good: gives 100 usec break and 16 usec MAB
// 1990 spec says transmitter must send >= 92 usec break and >= 12 usec MAB
// receiver must accept 88 us break and 8 us MAB
//#define BREAKSPEED   100000
#define BREAKSPEED     100000
#define DMXSPEED       250000
#define BREAKFORMAT    SERIAL_8E1
#define DMXFORMAT      SERIAL_8N2

// ----- Enumerations -----
//
//// State of receiving DMX Bytes
//typedef enum {
//  IDLE, BREAK, START, RUN
//} DMXReceivingState;
enum
{
	DMX_IDLE,
	DMX_BREAK,
	DMX_START,
	DMX_RUN
};

// ----- Macros -----
// calculate prescaler from baud rate and cpu clock rate at compile time
// nb implements rounding of ((clock / 16) / baud) - 1 per atmega datasheet
#define Calcprescale(B)     ( ( (((F_CPU)/8)/(B)) - 1 ) / 2 )

// ----- DMXSerial Private variables -----
// These variables are not class members because they have to be reached by the interrupt implementations.
// don't use these variable from outside, use the appropriate methods.

volatile DMXMode  _dmxMode; //  Mode of Operation

volatile uint8_t _dmxRecvState;  // Current State of receiving DMX Bytes
volatile int     _dmxChannel;  // the next channel byte to be sent.
volatile int     _dmxMaxChannel = 32; // the last channel used for sending (1..32).
volatile unsigned long _gotLastPacket = 0; // the last time (using the millis function) a packet was received.

// Array of DMX values (raw).
// Entry 0 will never be used for DMX data but will store the startbyte (0 for DMX mode).
// GS: Disabled the _dmxData so the RFPacket format can be used
//volatile uint8_t *  _dmxData;


// Buffer for the RFShowControl packet Structure.
byte str[18][32];

// Create a single class instance. Multiple class instances (multiple simultaneous DMX ports) are not supported.
ModifiedDMXSerialClass ModifiedDMXSerial;


volatile unsigned char dmx_state;
// this is the start address for the dmx frame
volatile unsigned int dmx_start_addr = 1;
// this is the current address of the dmx frame
volatile unsigned int dmx_addr;
// this is used to keep track of the channels
volatile unsigned int chan_cnt;
volatile unsigned int sub1 = 0;
volatile unsigned int sub2 = 0;

// tell us when to update
volatile unsigned char update;
volatile bool packetready = false;

int rxStatusPin = 0;
// ----- forwards -----

void _DMXSerialBaud(uint16_t baud_setting, uint8_t format);
void _DMXSerialWriteByte(uint8_t data);


// ----- Class implementation -----

// (Re)Initialize the specified mode.
// The mode parameter should be a value from enum DMXMode.
void ModifiedDMXSerialClass::init(int mode)
{
	// Setup external mode signal
	//TODO Refactor to constant A0 is used as the RX/TX en pin on the RF1 Serial adapter
	pinMode(A0, OUTPUT); // enables pin 2 for output to control data direction
	digitalWrite(A0, LOW); //Set it low to put the RS485 chip in Receive mode

#ifdef SCOPEDEBUG
	//if in debug mode define some pins for watching the timing in the logic analyzer
	pinMode(rxStatusPin, OUTPUT); // enables pin 2 for output to control data direction
	pinMode(DmxTriggerPin, OUTPUT);
	pinMode(DmxISRPin, OUTPUT);
#endif

	// initialize global variables
	_dmxMode = DMXNone;
	_dmxRecvState = DMX_IDLE; // initial state
	_dmxChannel = 0;
	_gotLastPacket = millis(); // remember current (relative) time in msecs.

	// initialize the DMX buffer
	for (int n = 0; n < 18; n++){
		for (int in = 0; in < 32; in++)
			str[n][in] = 0;
	}

	// now start
	_dmxMode = (DMXMode)mode;

	// Setup Hardware
	// Enable receiver and Receive interrupt
	UCSRnB = (1 << RXENn) | (1 << RXCIEn);
	_DMXSerialBaud(Calcprescale(DMXSPEED), DMXFORMAT); // Enable serial reception with a 250k rate
} // init()


// Set the maximum used channel.
// This method can be called any time before or after the init() method.
void ModifiedDMXSerialClass::maxChannel(int channel)
{
	if (channel < 1) channel = 1;
	if (channel > DMXSERIAL_MAX) channel = DMXSERIAL_MAX;
	_dmxMaxChannel = channel;
} // maxChannel


//TODO, Pull the RF packet writing out of the main loop so we dont have to hand out
//these pointers
byte * ModifiedDMXSerialClass::GetPacketPointer(void){
	return &str[sub1 - 1][0];
}

//TODO refactor me
bool ModifiedDMXSerialClass::isPacketReady(void){
	return packetready;
}

void ModifiedDMXSerialClass::setPacketReady(bool in){
	packetready = in;
}

// Terminale operation
void ModifiedDMXSerialClass::term(void)
{
	// Disable all USART Features, including Interrupts
	UCSRnB = 0;
} // term()


// ----- internal functions and interrupt implementations -----


// Initialize the Hardware serial port with the given baud rate
// using 8 data bits, no parity, 2 stop bits for data
// and 8 data bits, even parity, 1 stop bit for the break
void _DMXSerialBaud(uint16_t baud_setting, uint8_t format)
{
	// assign the baud_setting to the USART Baud Rate Register
	UCSRnA = 0;                 // 04.06.2012: use normal speed operation
	UBRRnH = baud_setting >> 8;
	UBRRnL = baud_setting;

	// 2 stop bits and 8 bit character size, no parity
	UCSRnC = format;
} // _DMXSerialBaud


// Interrupt Service Routine, called when a byte or frame error was received.
ISR(USART_RX_vect)
{

	unsigned char status = UCSR0A;
	unsigned char data = UDR0;

	//ISR Refactored from Switch to preserve memory
	if (dmx_state == DMX_IDLE){
		if (status & (1 << FE0))
		{
			dmx_addr = 0;
			dmx_state = DMX_BREAK;  //Once we get a break, that means we are about to read a new DMX packet.
		}
	}
	else if (dmx_state == DMX_BREAK)
	{
		if (data == 0)
		{
			dmx_state = DMX_START;
		}
		else
		{
			dmx_state = DMX_IDLE;
		}
	}
	else if (dmx_state == DMX_START)
	{
		//New DMX frame
		dmx_addr++;
		//Logic to handle the first channel
		if (dmx_addr == dmx_start_addr)
		{

			chan_cnt = 0;  //Reset Channel Count
			sub1 = 0;  //Reset array subscripts to 0
			sub2 = 0;
			str[sub1][sub2++] = data;
			chan_cnt++;
			dmx_state = DMX_RUN;
		}
	}
	else if (dmx_state == DMX_RUN)
	{
		str[sub1][sub2++] = data;
		chan_cnt++;  //increment channel

		//If we just added the 30th byte, its time to prep this packet for send.
		if (sub2 == 30)
		{
			str[sub1][30] = sub1;   //set packet number in byte 30
			sub2 = 0;               //reset packet byte counter
			sub1++;               //increment packet number
			packetready = true;     //flag that packet is ready to transmit
		}
		else if (chan_cnt > DMX_NUM_CHANNELS)  //Once we have gotten all the channels we want, quit listening....
		{
			dmx_state = DMX_IDLE;
			if (sub2>0 && ((sub1 * 30 + sub2 )== DMX_NUM_CHANNELS))  //Check to see if we got here with a partial final packet we didn't yet send....  If we did, send it now.
			{
				sub2 = 0;
				str[sub1][30] = sub1;
				sub1 = 0;//we sent the last packet reset
				packetready = true;
			}
		}
	}
	else{
		dmx_state = DMX_IDLE;
	}
}
