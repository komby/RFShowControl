/*
 * Test Transmitter sketch.
 * 
 * Generates a continuous sequence of RF data as a transmitter for debugging purposes.
 *
 *  Created on: Mar 25, 2013
 *      Author: Greg Scull
 */

#ifndef TESTRANSMITTER_H_
#define TESTTRANSMITTER_H_

#include <RFPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/***************************  CONFIGURATION SECTION *************************************************/
//use channel 100
#define TRANSMIT_CHANNEL 100

//Setup  a RF pixel control 
//RF1 v.01 board uses Radio 9,10
RFPixelControl radio(8,7);
//kombyone due transmitter board radio settings.
//RFPixelControl radio(33,10);


#define RENARD_SERIAL_BAUD	57600
//How many pixels do you want to transmit data for
#define NUM_CHANNELS		21
/***************************  CONFIGURATION SECTION *************************************************/

#define RF_DELAY      2000

#define PAD 0x7D
#define SYNC 0x7E
#define ESCAPE 0x7F
#define COMMAND 0x80

bool sync;
bool status;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

byte packetData[32];

int renardRead( uint8_t *bytes, uint8_t byte_count );

void print_data(char *data)
{
	printf("--%3d:", 0);
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		printf(" 0x%02X", data[i] & 0xFF);
		if ( (i-7) % 8 == 0 )
		{
			printf("\n--%3d:", i);
		}
	}
	printf("\n");
}

void setup()
{
	Serial.begin(RENARD_SERIAL_BAUD);
	Serial.println("\n[RenardTransmitter test]\n");
    printf_begin();
    radio.Initalize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, RF24_250KBPS, 50 );
	delayMicroseconds(150);
    radio.printDetails();
}

void loop ()
{
  uint8_t bytes_read = 0;

  for (int i = 0; i < NUM_CHANNELS; i += 30)
  {
	  bytes_read = renardRead(packetData, (NUM_CHANNELS-i >= 30 ? 30 : (NUM_CHANNELS%30) ));
//	  print_data((char *)packetData);
	  //if we got less than expected,  fill the remainder with PADs for now 
	  if (bytes_read < 30)
	  {
	    for(int j=bytes_read;j<30;j++)
		{
	      packetData[j]=PAD;
	    }
	  }

	//With the next line, the data doesn't appear to make it to the receiver?!?
//	packetData[30]=bytes_read;
	radio.write_payload( &packetData[0], 32 );
	delayMicroseconds(RF_DELAY);
	status = radio.get_status();
	//With the following while loop, we get stuck forever after the first transmission
//	while (status & 0x01)
//	{
//		status = radio.get_status();
//	}
  }
}

void wait_for_serial()
{
    while ( ! Serial.available() > 0 ) { }
}

//thanks materdaddy
int renardReadBytes(uint8_t *bytes, uint8_t bytes_size)
{
  int in_byte = 0;
  int bytes_read;
 
  for ( bytes_read = 0; bytes_read < bytes_size; )
  {
	wait_for_serial();
    in_byte = Serial.read();
 
    switch (in_byte)
    {
      case(0x7E): // We saw the sync byte, start over!
        sync = true;
        return bytes_read;
 
      case(0x7D): // Skip the pad byte
        continue;
 
      case(0x7F): // Escape character, we need to read one more byte to get our actual data
		wait_for_serial();
        in_byte = Serial.read();

        switch (in_byte)
          {
            case(0x2F): // renard wants an 0x7D
              in_byte = 0x7D;
            case(0x30): // renard wants an 0x7E
              in_byte = 0x7E;
            case(0x31): // renard wants an 0x7F
              in_byte = 0x7F;
          }
        }
 
    bytes[bytes_read++] = in_byte;
  }

  return bytes_read;
}

int renardRead(uint8_t *bytes, uint8_t byte_count)
{
  int in_byte = 0;
 
  while ( ! sync )
  {
    wait_for_serial();
    in_byte = Serial.read();
    if ( in_byte == 0x7E ) // Sync byte signifies start of packet
      sync = true;
  }

  if ( sync )
  {
    sync = false;
    wait_for_serial();
	in_byte = Serial.read();
    if ( in_byte == 0x80 ) // Read from here
    {
      return renardReadBytes(bytes, byte_count);
    }
  }
 
  return 0;
}


#endif 
