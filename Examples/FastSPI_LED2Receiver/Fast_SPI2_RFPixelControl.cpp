/*
 * Fast_SPI2_RFPixelControl.cpp
 * This class is a wrappter of the Fast_SPI2  ws2801 pixel library.
 * 
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#include "Fast_SPI2_RFPixelControl.h"





Fast_SPI2_RFPixelControl::Fast_SPI2_RFPixelControl() {
    Serial.print("defaultConst\n");
}

Fast_SPI2_RFPixelControl::~Fast_SPI2_RFPixelControl() {
}
void Fast_SPI2_RFPixelControl::InitFastSPI(CLEDController *  pController) {
  //controller is passed in constructor from user.
  if ( pController > 0 ){
    this->_fastSPIController=pController ;
  }
  else{
     Serial.print("nullPointer\n");
  }
}
/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void Fast_SPI2_RFPixelControl::Paint(byte * colors , unsigned int count) {
	this->pixels = colors;
        this->_fastSPIController->showRGB(colors, count);
        delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
        
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void Fast_SPI2_RFPixelControl::Paint() {
    Serial.write("in rfpixelcontrol paint\n");
	this->_fastSPIController->showRGB((byte *)this->pixels, this->numberOfPixels);
        delay(1);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}


/**
 *  method to keep compatibility with the other Pixel types
 */
void Fast_SPI2_RFPixelControl::Start(void) {
  Serial.println("in rfpixelcontrol start");
	this->_fastSPIController->init();
}


/* Helper functions */

/**
 * Create a 24 bit color value from R,G,B
 *
 * @param r - red 0-255 value
 * @param g - green 0-255 value
 * @param b - blue 0-255 value
 *
 * @return the color in a 24 bit pattern
 */
uint32_t Fast_SPI2_RFPixelControl::Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r ;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

/**
 * Helper to initialize the string (make sure its working)
 * This may also be used for debugging...
 *
 * @Param c - the 24 bit RGB color value - this is the integer representation of a color
 * @param wait - the duration to wait after the color is set.
 */
// fill the dots one after the other with said color
// good for testing purposes
void Fast_SPI2_RFPixelControl::ColorWipe(uint32_t c, uint8_t wait) {
  int i;
  for (i=0; i < this->numberOfPixels; i++) {
      this->SetPixelColor(i, c);
      this->Paint();
      delay(wait);
  }
}


        // this method may be helpful ... its from teh adafruit library.  
	//currently im storing the pixel info in rgb order,  since these pixels are crazy
	//and use BGR  it would be more efficient ( but less sane ) to store them in that order 
	//so that we can simply operate on the 24bit string ... this could allow us to not use
	//three registers in the assembly operation ( i think )
	// Set pixel color from 'packed' 32-bit RGB value:
	void  Fast_SPI2_RFPixelControl::SetPixelColor(uint16_t n, uint32_t c) {
			byte *p = &pixels[n * 3];
			// To keep the show() loop as simple & fast as possible, the
			// internal color representation is native to different pixel
			// types.  For compatibility with existing code, 'packed' RGB
			// values passed in or out are always 0xBBGGRR order.
  
			*p = c >> 16; // Red
			*++p = c >>  8; // Green
                        *++p = c;         // Blue
                       
	}
       // Set pixel color from separate 8-bit R, G, B components:
	// n is the pixel index (0-19) - its a bit backwards so,  pixel 0 is the 
	// last pixel in the string.  
	// r, g, b params are 8 bit RGB value
	// 
	//  For example:  RGB r=255,g=255, b=255
	//  in binary :  11111111 11111111 11111111
	//                RGB r=128, g=128, b=128
	//               10000000 10000000 10000000
	// 
	//  the most significant bit in each byte control the color being on
	//   so:  10000000  is less intense than 10000001
	//
	//  TODO research more about color mixing and support for colors.
	void Fast_SPI2_RFPixelControl::SetPixelColor(uint16_t n, byte r, byte g, byte b) {
		if(n < numberOfPixels) { // Arrays are 0-indexed, thus NOT '<='
			byte *p = &pixels[n * 3];
			*p++ = r;
			*p++ = g;
			*p = b;
	  	}
	}
// Query color from previously-set pixel (returns packed 32-bit RGB value)
	// TODO:  Test this function,  Unsure if it is working correctly.
	//		  
	uint32_t Fast_SPI2_RFPixelControl::GetPixelColor(uint16_t n) {
		if(n < this->numberOfPixels) {
			uint16_t ofs = n * 3;
			// To keep the show() loop as simple & fast as possible, the
			// internal color representation is native to different pixel
			// types.  For compatibility with existing code, 'packed' RGB
			// values passed in or out are always 0xRRGGBB order.
			return 
				((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2];
		}
		return 0;
	}

       //Accessor method to expose the number of pixels in the string
	uint32_t Fast_SPI2_RFPixelControl::GetPixelCount(void) {
		return this->numberOfPixels;

	}

	//Mutator to set the number of pixels in the string
	void Fast_SPI2_RFPixelControl::SetPixelCount(  uint32_t pCount) {
		alloc(pCount);
               this->numberOfPixels = pCount;

	}


	// Handle memory allocation for the entire strings worth of data.
	// This allocation will be a continuous "packed" section of memory
	// The pixels pointer in this class is the base address of that memory location
	void Fast_SPI2_RFPixelControl::alloc(uint16_t n) {
     
		// If we already have some memory allocated, free it before getting more.
  		if (pixels != NULL) {
			free(pixels);
		}
		// If something goes wrong with the allocation 
		// the pixel string will be set to length 0
		numberOfPixels = ((pixels = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
       
       }
 
