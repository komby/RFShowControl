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
#define TRANSMIT_CHANNEL 100

//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

#define RENARD_SERIAL_BAUD	57600

//What board are you using to connect your nRF24L01+?
//Valid Values: MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1,KOMBYONE_DUE,
#define NRF_TYPE			RF1_1_3

#define RENARD_BAUD_RATE	57600

#define START_CHANNEL 		0
#define NUM_CHANNELS 		400


//*****************************************************************************

//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>



#define RF_DELAY      2000

#define PAD 0x7D
#define SYNC 0x7E
#define ESCAPE 0x7F
#define COMMAND 0x80

bool sync;
bool status;


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
	  //radio.Initalize( radio.TRANSMITTER, pipes, 100 , RF24_1MBPS , 0);
    radio.Initialize( radio.TRANSMITTER, pipes, TRANSMIT_CHANNEL, DATA_RATE, 0 );
	delayMicroseconds(150);
    radio.printDetails();
}

void loop ()
{
  uint8_t bytes_read = 0;

  for (int i = 0; i < NUM_CHANNELS; i += 30)
  {
	  bytes_read = renardRead(packetData, (NUM_CHANNELS-i >= 30 ? 30 : (NUM_CHANNELS%30) ));
	  //if we got less than expected,  fill the remainder with PADs for now 
	  if (bytes_read < 30)
	  {
	    for(int j=bytes_read;j<30;j++)
		{
	      packetData[j]=PAD;
	    }
	  }
	radio.write_payload( &packetData[0], 32 );
	delayMicroseconds(RF_DELAY);
	status = radio.get_status();
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
