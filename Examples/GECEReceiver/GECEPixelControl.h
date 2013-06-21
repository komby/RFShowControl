/*
 * GECEPixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 *
 *      This class is built as an adapter to control the GECE pixels with the RFPixel Control library.
 *      It is using a library which was written by Adafruit as the main pixel logic
 *      @see https://github.com/adafruit/Adafruit-GECE-Library/ for more details
 *
 */

#ifndef GECEPIXELCONTROL_H_
#define GECEPIXELCONTROL_H_

#include <GEColorEffects.h>
#include <IPixelControl.h>
#include <RFPixelControl.h>

class GECEPixelControl:public GEColorEffects, public IPixelControl
{
public:
  GECEPixelControl();
  GECEPixelControl(int pixels, int pin);
  ~GECEPixelControl();

  void Paint(uint8_t * colors, unsigned int count);
  void Paint(void);
  uint32_t GetPixelCount(void);
  void SetPixelCount(uint32_t);
  void Start(void);
  uint32_t GetPixelColor(uint16_t n);

  void SetPixelColor(uint16_t n, uint32_t c);
  void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  uint32_t Color(byte r, byte g, byte b);
  void ColorWipe(uint32_t c, uint8_t wait);
private:
  uint8_t *pixels;
  uint16_t numLEDs;
  uint8_t dataPin;
};

#endif /* GECEPIXELCONTROL_H_ */
