/*
 * GWTSControl.h
 *
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 *
 *      This class is built as an adapter to control the GWTS Mickey Ears with the RFPixelControl library.
 *
 */

#ifndef GWTSCONTROL_H_
#define GWTSCONTROL_H_
#include <Arduino.h>
#include <GwtS.h>
#include <IPixelControl.h>
#include <RFPixelControl.h>

class GWTSControl: public GWTS, public IPixelControl
{
public:
	GWTSControl();
	
	void Paint(void);
};

#endif /* GWTSCONTROL_H_ */
