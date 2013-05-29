/*
 * LPD6803PixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 *
 *      This class is built as an adapter to control the LPD6803 pixels with the RFPixel Control library.
 *      It is using a library which was written by Adafruit as the main pixel logic
 *      @see https://github.com/adafruit/Adafruit-LPD6803-Library/ for more details
 *
 */

#ifndef ADAFRUITILIB6803
#define ADAFRUITILIB6803
#include <LPD6803.h>
#include <TimerOne.h>
#include <IPixelControl.h>
#include <RFPixelControl.h>
class LPD6803PixelControl : public LPD6803, public IPixelControl{
public:
	LPD6803PixelControl();
	 ~LPD6803PixelControl();

		void Paint(uint8_t * colors , unsigned int count);
		void  Paint(void);
		uint32_t  GetPixelCount( void );
		void  SetPixelCount( uint32_t );
		void  Start( void );
		uint32_t  GetPixelColor(uint16_t n);

		void  SetPixelColor(uint16_t n, uint32_t c);
		void  SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
        uint32_t Color(byte r, byte g, byte b);
        void ColorWipe(uint32_t c, uint8_t wait);
};

#endif /* LPD6803PIXELCONTROL_H_ */
