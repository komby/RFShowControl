/*
 * GECERFShowControl - This class is built as an adapter to control the GECE pixels with the RFShow Control library.
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

#ifndef __GECERFSHOWCONTROL_H__
#define __GECERFSHOWCONTROL_H__

#include <Arduino.h>
#include "GEColorEffects.h"

#include "IRFShowControl.h"
#include "RFShowControl.h"

class GECERFShowControl: public GEColorEffects, public IRFShowControl
{
public:
  GECERFShowControl(uint8_t pin, int pixelCount, bool pInvertOutputOrder = false);
  void Paint(void);
};

#endif //__GECERFSHOWCONTROL_H__
