/*
 * Fast_SPI2_RFPixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 *
 *      This class is built as an adapter to control pixels with the FastSPI2 controllers 
 *      in coordination with the RFPixel Control library.
 *
 *      It is using the Fast_SPI2 library 
 *      @see https://code.google.com/p/fastspi/ for more details
 *
 */

#ifndef FAST_SPI2_RFPIXELCONTROL_H_
#define FAST_SPI2_RFPIXELCONTROL_H_

#include <IPixelControl.h>
#include <RFPixelControl.h>
#include <FastSPI_LED2.h>
#include <controller.h>

// Hardware SPI - .652ms for an 86 led frame @8Mhz (3.1Mbps?), .913ms @4Mhz 1.434ms @2Mhz
// With the wait ordering reversed,  .520ms at 8Mhz, .779ms @4Mhz, 1.3ms @2Mhz
// WS2801Controller<11, 13, 10, 0> LED;

// Same Port, non-hardware SPI - 1.2ms for an 86 led frame, 1.12ms with large switch 
// WS2801Controller<11, 13, 10, 0> LED;

// Different Port, non-hardware SPI - 1.47ms for an 86 led frame
// WS2801Controller<7, 13, 10> LED;




//struct to store the pixel color information
//this is a packed struct such that the memory
//is contigous allowing for pointer iteration.
typedef struct __attribute__ ((__packed__)) 
{
	uint8_t  r, g, b;
} rgb_color;



class Fast_SPI2_RFPixelControl : public IPixelControl{
public:
	        Fast_SPI2_RFPixelControl();

	        ~Fast_SPI2_RFPixelControl();
               void  InitFastSPI( CLEDController *  pCledController);                
                 void alloc(uint16_t n);
		void Paint(byte * colors , unsigned int count);
		void  Paint(void);
		uint32_t  GetPixelCount( void );
		void  SetPixelCount( uint32_t );
		void  Start( void );
		uint32_t  GetPixelColor(uint16_t n);

		void  SetPixelColor(uint16_t n, uint32_t c);
		void  SetPixelColor(uint16_t n, byte r, byte g, byte b);
                uint32_t Color(byte r, byte g, byte b);
                void ColorWipe(uint32_t c, uint8_t wait);

                int numberOfPixels;   //number of pixels
	        byte * pixels;		   //Pointer to the base address of the pixel colors in memory
                CLEDController * _fastSPIController;
};

#endif /* Fast_SPI2_RFPixelControl_H_ */
