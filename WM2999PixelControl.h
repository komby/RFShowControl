/*
 * WM2999PixelControl.h
 * This class is built as an adapter to control the WM2999 pixels with the RFPixel Control library.
 * It is using a library which was written by Adafruit as the main pixel logic
 * @see https://github.com/komby/WM2999 for more details
 *
 * Created on: Mar 19, 2013
 * Author: Greg Scull - komby@komby.com
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

#ifndef __WM2999PIXELCONTROL_H__
#define __WM2999PIXELCONTROL_H__

#include <WM2999.h>

#include "IPixelControl.h"
#include "RFPixelControl.h"

class WM2999PixelControl: public WM2999, public IPixelControl
{
public:
	WM2999PixelControl(uint8_t ppin);
	void Begin(uint8_t *pDataPointer, int pNumLEDs);

	void SetDataBasePointer(uint8_t *dataPointer);

	void Paint(void);
};

#endif //__WM2999PIXELCONTROL_H__
