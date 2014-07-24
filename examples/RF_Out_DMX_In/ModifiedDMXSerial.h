// - - - - -
// DMXSerial - A hardware supported interface to DMX.
// DMXSerial.h: Library header file
//
// Copyright (c) 2011 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
//
// Documentation and samples are available at http://www.mathertel.de/Arduino
//
// 17.03.2014 Greg Scull modified for the use in RFShowControl.
// - - - - -

#ifndef ModifiedDmxSerial_h
#define ModifiedDmxSerial_h

#include <avr/io.h>

// ----- Constants -----

#define DMXSERIAL_MAX 512 // max. number of supported DMX data channels

#define DmxModePin A0     // Arduino pin A0 for controlling the data direction
#define DmxModeOut HIGH  // set the level to HIGH for outgoing data direction
#define DmxModeIn  LOW   // set the level to LOW  for incoming data direction

// ----- Enumerations -----

// Mode of Operation
typedef enum {
  DMXNone, // unspecified
  DMXController, // always sending
  DMXReceiver, // always listening
  RDMTestmode // not implemented yet
} DMXMode;  // not implemented yet.


// ----- Library Class -----

class ModifiedDMXSerialClass
{
  public:
  // Initialize for specific mode.
  void    init       (int mode);

  // Set the maximum used channel for DMXController mode.
  void    maxChannel (int channel);

  // Read the last known value of a channel.
  uint8_t read       (int channel);

  // Write a new value of a channel.
  void    write      (int channel, uint8_t value);

  // Calculate how long no data packet was received
  unsigned long noDataSince(void);

  // Terminate operation.
  void    term       (void);

  bool isPacketReady(void);

  void setPacketReady(bool in);
  byte * GetPacketPointer(void);
};

// Use the DMXSerial library through the DMXSerial object.
extern ModifiedDMXSerialClass ModifiedDMXSerial;

#endif
