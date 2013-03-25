/*
 * WS2801PixelControl.cpp
 * This class is a wrappter of the Adafruit ws2801 pixel library.
 * 
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#include "WS2801PixelControl.h"

WS2801PixelControl::WS2801PixelControl() {
	//for now expect 50 pixel strings on rx and tx pins.
	 //this->adaPixels =   new Adafruit_WS2801(50, 2, 3);
	this->updatePins(2,3);
        this->updateOrder(0); 


}

WS2801PixelControl::~WS2801PixelControl() {
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void WS2801PixelControl::Paint(uint8_t * colors , unsigned int count) {
	this->show();
        delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
        
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void WS2801PixelControl::Paint() {
	this->show();
        delay(2);  //Hold for 2 milliseconds to latch color. This may be 1MS longer than needed?? 
}


uint32_t WS2801PixelControl::GetPixelCount(void) {
	
	return this->numPixels();
}

void WS2801PixelControl::SetPixelCount(uint32_t count) {
	updateLength(count);
}

/**
 *  method to keep compatibility with the other Pixel types
 */
void WS2801PixelControl::Start(void) {
	this->begin();
}

uint32_t WS2801PixelControl::GetPixelColor(uint16_t n) {
	getPixelColor(n);
}

void WS2801PixelControl::SetPixelColor(uint16_t n, uint32_t c) {
	setPixelColor(n, c);
}

void WS2801PixelControl::SetPixelColor(uint16_t n, uint8_t r, uint8_t g,
		uint8_t b) {
	this->setPixelColor(n, r, g, b);
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
uint32_t WS2801PixelControl::Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
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
void WS2801PixelControl::ColorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < this->numPixels(); i++) {
      this->SetPixelColor(i, c);
      this->Paint();
      delay(wait);
  }
}
