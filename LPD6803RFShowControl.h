/*
 * LPD6803RFShowControl.h
 * This class is built as an adapter to control the LPD6803 pixels with the RFShow Control library.
 * It is using a library which was written by Adafruit as the main pixel logic
 * @see https://github.com/adafruit/Adafruit-LPD6803-Library/ for more details
 *
 * Created on: Mar 19, 2013
 * Author: Greg Scull
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

#ifndef __LPD6803RFSHOWCONTROL_H__
#define __LPD6803RFSHOWCONTROL_H__

#include <TimerOne.h>

#include "IRFShowControl.h"
#include "LPD6803.h"
#include "RFShowControl.h"

class LPD6803RFShowControl: public IRFShowControl, public LPD6803
{
public:
  LPD6803RFShowControl(uint8_t pClock, uint8_t pData);

  void Paint(void);
};

#endif /* __LPD6803RFSHOWCONTROL_H__ */
