/*
 * WM2999PixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg Scull - komby@komby.com
 *
 *      This class is built as an adapter to control the WM2999 pixels with the RFPixel Control library.
 *      It is using a library which was written by Adafruit as the main pixel logic
 *      @see https://github.com/komby/WM2999 for more details
 *
 */

#ifndef WM2999PIXELCONTROL_H_
#define WM2999PIXELCONTROL_H_

#include <IPixelControl.h>
#include <RFPixelControl.h>
#include <WM2999.h>


class WM2999PixelControl : public WM2999, public IPixelControl
{
public:
	WM2999PixelControl(uint8_t ppin);

	void Paint(void);
};

#endif /* WM2999PIXELCONTROL_H_ */
