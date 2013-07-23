/*
 * RenardControl.h
 *
 *  Created on: June 20th, 2013
 *      Author: Mat Mrosko
 *
 *      This class is built as an adapter to control the Renard devices with the RFPixel Control library.
 *
 */

#ifndef RENARDCONTROL_H_
#define RENARDCONTROL_H_
#include <Arduino.h>
#include <IPixelControl.h>
#include <RFPixelControl.h>

class RenardControl: public IPixelControl
{
public:
	RenardControl(uint32_t baud_rate);
	
	void Paint(void);
};

#endif /* RENARDCONTROL_H_ */
