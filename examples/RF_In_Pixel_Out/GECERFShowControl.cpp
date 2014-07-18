/*
 * GECERFShowControl: Wrappter of the GEColorEffects library for IRFShowControl
 *
 * Created on: June 20th, 2013
 * Author: Mat Mrosko
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

#include "GECERFShowControl.h"

GECERFShowControl::GECERFShowControl(uint8_t pin, int pixelCount) : GEColorEffects(pin,pixelCount)
{
}

/**
 * Paint method to keep compatibility with the other Pixel types
 */
void GECERFShowControl::Paint(void)
{
  cli();
  int numPixels = numLEDs/3;
  for ( int i = 0; i < numPixels; i++ )
  {
    this->set_color(i,
                    DEFAULT_INTENSITY,
                    this->color((pixels[(i*3)])>>4,
                                (pixels[(i*3)+1])>>4,
                                 pixels[(i*3)+2]>>4
                                ));
  }

  sei();
}
