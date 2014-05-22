/*
 * GECEPixelControl - This class is built as an adapter to control the GECE pixels with the RFPixel Control library.
 *
 * Created on: June 20th, 2013
 * Author: Mat Mrosko
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * License:
 *		Users of this software agree to hold harmless the creators and
 *		contributors of this software.  By using this software you agree that
 *		you are doing so at your own risk, you could kill yourself or someone
 *		else by using this software and/or modifying the factory controller.
 *		By using this software you are assuming all legal responsibility for
 *		the use of the software and any hardware it is used on.
 *
 *		The Commercial Use of this Software is Prohibited.
 */

#ifndef __GECEPIXELCONTROL_H__
#define __GECEPIXELCONTROL_H__

#include <Arduino.h>
#include <GEColorEffects.h>

#include "IPixelControl.h"
#include "RFPixelControl.h"

class GECEPixelControl: public GEColorEffects, public IPixelControl
{
public:
	GECEPixelControl(uint8_t pin, int pixelCount);
	void Paint(void);
};

#endif //__GECEPIXELCONTROL_H__
