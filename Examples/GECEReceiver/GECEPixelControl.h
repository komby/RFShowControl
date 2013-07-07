/*
 * GECEPixelControl.h
 *
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 *
 *      This class is built as an adapter to control the GECE pixels with the RFPixel Control library.
 *
 */

#ifndef GECEPIXELCONTROL_H_
#define GECEPIXELCONTROL_H_
#include <Arduino.h>
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
