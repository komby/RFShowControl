/*
 * FastSPI_LED2_PixelControl - This class is built as an adapter to control all of the pixels supported by FastSPI_LED2 with the RFPixel Control library.
 *
 * Created on: May 28th, 2014
 * Author: Mat Mrosko (Materdaddy) rfpixelcontrol@matmrosko.com
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

#ifndef __FASTSPI_LED2_PIXELCONTROL_H__
#define __FASTSPI_LED2_PIXELCONTROL_H__

#include <Arduino.h>
#include <FastSPI_LED2.h>

#include "IPixelControl.h"
#include "RFPixelControl.h"

class FastSPI_LED2_PixelControl: public IPixelControl
{
public:
	FastSPI_LED2_PixelControl(CLEDController *pLed);
	void Paint(void);
	void Begin(RFPixelControl *radio);
private:
	CRGB *leds;
	CLEDController *mLed;
};

#endif //__FASTSPI_LED2_PIXELCONTROL_H__
