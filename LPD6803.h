/*
 * Author: Greg Scull/Mat Mrosko
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * License:
 *    Users of this software agree to hold harmless the creators and
 *    contributors of this software.  By using this software you agree that
 *    you are doing so at your own risk, you could kill yourself or someone
 *    else by using this software and/or modifying the factory controller.
 *    By using this software you are assuming all legal responsibility for
 *    the use of the software and any hardware it is used on.
 *
 *    The Commercial Use of this Software is Prohibited.
 */

#ifndef __LPD6803_H__
#define __LPD6803_H__

#include <Arduino.h>

class LPD6803
{
private:
  uint8_t cpumax;

public:
  LPD6803(uint16_t n, uint8_t dpin, uint8_t cpin);
  void begin(void);
  void show(void);
  void doSwapBuffersAsap(uint16_t idx);
  void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  void setPixelColor(uint16_t n, uint16_t c);
  void setCPUmax(uint8_t m);
  uint16_t numPixels(void);
};

#endif //__LPD6803_H__
