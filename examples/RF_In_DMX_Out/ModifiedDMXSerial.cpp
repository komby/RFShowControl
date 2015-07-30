// - - - - -
// DMXSerial - A hardware supported interface to DMX.
// DMXSerial.cpp: Library implementation file
//
// Copyright (c) 2011 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
//
// Documentation and samples are available at http://www.mathertel.de/Arduino
// 25.07.2011 creation of the DMXSerial library.
// 10.09.2011 fully control the serial hardware register
//            without using the Arduino Serial (HardwareSerial) class to avoid ISR implementation conflicts.
// 01.12.2011 include file changed to work with the Arduino 1.0 environment
// 28.12.2011 unused variable DmxCount removed
// 10.05.2012 added method noDataSince to check how long no packet was received
// 04.06.2012: set UCSRnA = 0 to use normal speed operation
// 30.07.2012 corrected TX timings with UDRE and TX interrupts
//            fixed bug in 512-channel RX

// - - - - -

#include <Arduino.h>
#include <avr/interrupt.h>

#include "ModifiedDMXSerial.h"

// ----- Debugging -----

// to debug on an oscilloscope, enable this
#undef SCOPEDEBUG
#ifdef SCOPEDEBUG
#define DmxTriggerPin 9  // low spike at beginning of start byte
#define DmxISRPin 3  // low during interrupt service routines
#endif

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
#define BREAKSPEED     100000
#define DMXSPEED       250000
#define BREAKFORMAT    SERIAL_8E1
#define DMXFORMAT      SERIAL_8N2

// ----- Enumerations -----

// State of receiving DMX Bytes
typedef enum {
  IDLE, BREAK, DATA
} DMXReceivingState;

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
uint8_t stat = 0 ;
// Array of DMX values (raw).
// Entry 0 will never be used for DMX data but will store the startbyte (0 for DMX mode).
volatile uint8_t *  _dmxData;

// Create a single class instance. Multiple class instances (multiple simultaneous DMX ports) are not supported.
ModifiedDMXSerialClass ModifiedDMXSerial;


// ----- forwards -----

void _DMXSerialBaud(uint16_t baud_setting, uint8_t format);
void _DMXSerialWriteByte(uint8_t data);


// ----- Class implementation -----

// (Re)Initialize the specified mode.
// The mode parameter should be a value from enum DMXMode.
void ModifiedDMXSerialClass::init (int mode, uint8_t* data)
{
  stat = 0;
  #ifdef SCOPEDEBUG
  pinMode(DmxTriggerPin, OUTPUT);
  pinMode(DmxISRPin, OUTPUT);
  #endif
  _dmxData = data;
  // initialize global variables
  _dmxMode = DMXNone;
  _dmxRecvState= IDLE; // initial state
  _dmxChannel = 0;
  _gotLastPacket = millis(); // remember current (relative) time in msecs.

  // initialize the DMX buffer
  for (int n = 0; n < DMXSERIAL_MAX+1; n++)
  _dmxData[n] = 0;

  // now start
  _dmxMode = (DMXMode)mode;

  if (_dmxMode == DMXController) {
    // Setup external mode signal
    pinMode(DmxModePin, OUTPUT); // enables pin 2 for output to control data direction
    digitalWrite(DmxModePin, DmxModeOut); // data Out direction

    // Setup Hardware
    // Enable transmitter and interrupt
    UCSRnB = (1<<TXENn) | (1<<TXCIEn);

    // Start sending a BREAK and loop (forever) in UDRE ISR
    _DMXSerialBaud(Calcprescale(BREAKSPEED), BREAKFORMAT);
    _DMXSerialWriteByte((uint8_t)0);
    _dmxChannel = 0;

    } else if (_dmxMode == DMXReceiver) {
    // Setup external mode signal
    pinMode(DmxModePin, OUTPUT); // enables pin 2 for output to control data direction
    digitalWrite(DmxModePin, DmxModeIn); // data in direction

    // Setup Hardware
    // Enable receiver and Receive interrupt
    UCSRnB = (1<<RXENn) | (1<<RXCIEn);
    _DMXSerialBaud(Calcprescale(DMXSPEED), DMXFORMAT); // Enable serial reception with a 250k rate

    } else {
    // Enable receiver and transmitter and interrupts
    // UCSRnB = (1<<RXENn) | (1<<TXENn) | (1<<RXCIEn) | (1<<UDRIEn);

  } // if
} // init()


// Set the maximum used channel.
// This method can be called any time before or after the init() method.
void ModifiedDMXSerialClass::maxChannel(int channel)
{
  if (channel < 1) channel = 1;
  if (channel > DMXSERIAL_MAX) channel = DMXSERIAL_MAX;
  _dmxMaxChannel = channel;
} // maxChannel


// Read the current value of a channel.
uint8_t ModifiedDMXSerialClass::read(int channel)
{
  // adjust parameter
  if (channel < 1) channel = 1;
  if (channel > DMXSERIAL_MAX) channel = DMXSERIAL_MAX;
  // read value from buffer
  return(_dmxData[channel]);
} // read()


// Write the value into the channel.
// The value is just stored in the sending buffer and will be picked up
// by the DMX sending interrupt routine.
void ModifiedDMXSerialClass::write(int channel, uint8_t value)
{
  // adjust parameters
  if (channel < 1) channel = 1;
  if (channel > DMXSERIAL_MAX) channel = DMXSERIAL_MAX;
  if (value < 0)   value = 0;
  if (value > 255) value = 255;

  // store value for later sending
  _dmxData[channel] = value;
} // write()


// Calculate how long no data packet was received
unsigned long ModifiedDMXSerialClass::noDataSince(void)
{
  unsigned long now = millis();
  return(now - _gotLastPacket);
} // noDataSince()


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

// send the next byte after current byte was sent completely.
void _DMXSerialWriteByte(uint8_t data)
{
  // putting data into buffer sends the data
  UDRn = data;
} // _DMXSerialWrite


// Interrupt Service Routine, called when a byte or frame error was received.
ISR(USART_RX_vect)
{
  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, LOW);
  #endif
  //  digitalWrite(rxStatusPin, HIGH);
  uint8_t  USARTstate= UCSRnA;    //get state before data!
  uint8_t  DmxByte   = UDRn;      //get data
  uint8_t  DmxState  = _dmxRecvState;  //just load once from SRAM to increase speed

  if (USARTstate & (1<<FEn)) {    //check for break
    _dmxRecvState = BREAK; // break condition detected.
    _dmxChannel= 0;        // The next data byte is the start byte

    } else if (DmxState == BREAK) {
    if (DmxByte == 0) {
      digitalWrite(1, stat);
      stat= !stat;
      #ifdef SCOPEDEBUG
      digitalWrite(DmxTriggerPin, LOW);
      digitalWrite(DmxTriggerPin, HIGH);
      #endif
      _dmxRecvState = DATA;  // normal DMX start code detected
      _dmxChannel= 1;       // start with channel # 1
      _gotLastPacket = millis(); // remember current (relative) time in msecs.

      } else {
      // This might be a RDM command -> not implemented so wait for next BREAK !
      _dmxRecvState= IDLE;
    } // if

    } else if (DmxState == DATA) {
    _dmxData[_dmxChannel]= DmxByte;  // store received data into dmx data buffer.
    _dmxChannel++;
    if (_dmxChannel > DMXSERIAL_MAX) { // all channels done.
      _dmxRecvState = IDLE;  // wait for next break
    } // if

  } // if

  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, HIGH);
  #endif
} // ISR(USART_RX_vect)


// Interrupt service routines that are called when the actual byte was sent.
// When changing speed (for sending break and sending start code) we use TX finished interrupt
// which occurs shortly after the last stop bit is sent
// When staying at the same speed (sending data bytes) we use data register empty interrupt
// which occurs shortly after the start bit of the *previous* byte
// When sending a DMX sequence it just takes the next channel byte and sends it out.
// In DMXController mode when the buffer was sent completely the DMX sequence will resent, starting with a BREAK pattern.
// In DMXReceiver mode this interrupt is disabled and will not occur.
// In RDM mode this interrupt acts like in DMXController mode but the packet is not resent automatically.
ISR(USART_TX_vect)
{
  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, LOW);
  #endif
  if ((_dmxMode == DMXController) && (_dmxChannel == -1)) {
    // this interrupt occurs after the stop bits of the last data byte
    // start sending a BREAK and loop forever in ISR
    _DMXSerialBaud(Calcprescale(BREAKSPEED), BREAKFORMAT);
    _DMXSerialWriteByte((uint8_t)0);
    _dmxChannel = 0;

    } else if (_dmxChannel == 0) {
    // this interrupt occurs after the stop bits of the break byte
    // now back to DMX speed: 250000baud
    _DMXSerialBaud(Calcprescale(DMXSPEED), DMXFORMAT);
    // take next interrupt when data register empty (early)
    UCSRnB = (1<<TXENn) | (1<<UDRIEn);
    // write start code
    _DMXSerialWriteByte((uint8_t)0);
    _dmxChannel = 1;

    #ifdef SCOPEDEBUG
    digitalWrite(DmxTriggerPin, LOW);
    digitalWrite(DmxTriggerPin, HIGH);
    #endif
  } // if

  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, HIGH);
  #endif
} // ISR(USART_TX_vect)

// this interrupt occurs after the start bit of the previous data byte
ISR(USART_UDRE_vect)
{
  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, LOW);
  #endif
  _DMXSerialWriteByte(_dmxData[(_dmxChannel-1)]);
  _dmxChannel++;
  if (_dmxChannel > _dmxMaxChannel) {
    _dmxChannel   = -1; // this series is done. Next time: restart with break.
    // get interrupt after this byte is actually transmitted
    UCSRnB = (1<<TXENn) | (1<<TXCIEn);
  } // if
  #ifdef SCOPEDEBUG
  digitalWrite(DmxISRPin, HIGH);
  #endif
} // ISR(USART_UDRE_vect)