/*
 * GECEPixelControl.cpp
 * This class is a wrappter of the GEColorEffects library for an IPixelControl implementation.
 * 
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 */

#include "GECEPixelControl.h"

GECEPixelControl::GECEPixelControl() : GEColorEffects(50,3)
{
  numLEDs = ((pixels = (uint8_t *)calloc(50, 3)) != NULL) ? 50 : 0;
  dataPin=3;
}

GECEPixelControl::~GECEPixelControl()
{
  if (pixels != NULL)
    free(pixels);
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void
GECEPixelControl::Paint(uint8_t * colors, unsigned int count)
{
  int i;
  for ( i = 0; i < count; i++ )
  {
    this->set_color(i, DEFAULT_INTENSITY, this->color(
                                                      (colors[0])>>4,
                                                      (colors[1])>>4,
                                                       colors[2]>>4
                                                     ));
  }
}

/**
 * Paint method to keep compatibility with the other Pixel types
 * Call the adafruit method show to update the string.
 */
void
GECEPixelControl::Paint()
{
  int i;
  for ( i = 0; i < numLEDs; i++ )
  {
    this->set_color(i, DEFAULT_INTENSITY, this->color(
                                                      (pixels[(i*3)]<<16)>>4,
                                                      (pixels[(i*3)+1]<<8)>>4,
                                                       pixels[(i*3)+2]>>4
                                                     ));
  }
}


uint32_t
GECEPixelControl::GetPixelCount(void)
{
  return numLEDs;
}

void
GECEPixelControl::SetPixelCount(uint32_t count)
{
  if (pixels != NULL)
    free(pixels);
  numLEDs = ((pixels = (uint8_t *)calloc(count, 3)) != NULL) ? count : 0;
}

/**
 *  method to keep compatibility with the other Pixel types
 */
void
GECEPixelControl::Start(void)
{
  printf("Starting GECEs...\n");
}

uint32_t
GECEPixelControl::GetPixelColor(uint16_t n)
{
  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return ((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2];
  }

  return 0; // Pixel # is out of bounds
}

void
GECEPixelControl::SetPixelColor(uint16_t n, uint32_t c)
{
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    *p++ = c >> 16; // Red
    *p++ = c >>  8; // Green
    *p++ = c;         // Blue
  }
}

void
GECEPixelControl::SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // See notes later regarding color order
    *p++ = r;
    *p++ = g;
    *p++ = b;
  }
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
uint32_t
GECEPixelControl::Color(byte r, byte g, byte b)
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
 * Helper to initialize the string(make sure its working)
 * This may also be used for debugging...
 *
 * @Param c - the 24 bit RGB color value - this is the integer representation of a color
 * @param wait - the duration to wait after the color is set.
 */
// fill the dots one after the other with said color
// good for testing purposes
void
GECEPixelControl::ColorWipe(uint32_t c, uint8_t wait)
{
  int i;

  for(i = 0; i < numLEDs; i++)
    {
      this->SetPixelColor(i, c);
      this->Paint();
      delay(wait);
    }
}
