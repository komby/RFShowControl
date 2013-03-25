/*
 * IPixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#ifndef IPIXELCONTROL_H_
#define IPIXELCONTROL_H_
#include <Arduino.h>
//base class for pixels
class IPixelControl {
public:
	virtual ~IPixelControl() {};
	//	void virtual Paint(rgb_color *, unsigned int count) = 0;
	void virtual Paint(uint8_t * colors , unsigned int count) = 0;
	void virtual Paint(void) = 0;
	uint32_t virtual GetPixelCount( void ) = 0;
	void virtual SetPixelCount( uint32_t ) = 0;
	void virtual Start( void ) = 0;
	uint32_t virtual GetPixelColor(uint16_t n) = 0;
	//n is the index of previous color
	//	uint32_t virtual SetPixelColor(uint16_t n) = 0;
	void virtual SetPixelColor(uint16_t n, uint32_t c)  = 0;
	void virtual SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) = 0;
	void virtual ColorWipe(uint32_t c, uint8_t wait)  = 0;
	uint32_t virtual  Color(byte r, byte g, byte b) =0;
};
//define virtual destructor
//virtual IPixelControl::~IPixelControl(){}

#endif /* IPIXELCONTROL_H_ */

