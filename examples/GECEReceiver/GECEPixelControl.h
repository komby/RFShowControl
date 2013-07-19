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

class GECEPixelControl: public GEColorEffects, public IPixelControl
{
public:
	GECEPixelControl();

	void Paint(void);
};

#endif /* GECEPIXELCONTROL_H_ */
