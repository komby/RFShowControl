/*
 * Fast_SPI2_RFPixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 *
 *      This class is built as an adapter to control pixels with the FastSPI2 controllers 
 *      in coordination with the RFPixel Control library.
 *
 *      It is using the Fast_SPI2 library 
 *      @see https://code.google.com/p/fastspi/ for more details
 *
 */

#ifndef FAST_SPI2_RFPIXELCONTROL_H_
#define FAST_SPI2_RFPIXELCONTROL_H_

#include <IPixelControl.h>
#include <RFPixelControl.h>
#include <FastSPI_LED2.h>
#include <controller.h>

// Hardware SPI - .652ms for an 86 led frame @8Mhz (3.1Mbps?), .913ms @4Mhz 1.434ms @2Mhz
// With the wait ordering reversed, .520ms at 8Mhz, .779ms @4Mhz, 1.3ms @2Mhz
// WS2801Controller<11, 13, 10, 0> LED;

// Same Port, non-hardware SPI - 1.2ms for an 86 led frame, 1.12ms with large switch 
// WS2801Controller<11, 13, 10, 0> LED;

// Different Port, non-hardware SPI - 1.47ms for an 86 led frame
// WS2801Controller<7, 13, 10> LED;


class Fast_SPI2_RFPixelControl : public IPixelControl
{
public:
	void InitFastSPI( CLEDController * pCledController);

	void Paint(byte * colors , unsigned int count);
	void Paint(void);

	CLEDController * _fastSPIController;
};

#endif /* Fast_SPI2_RFPixelControl_H_ */
