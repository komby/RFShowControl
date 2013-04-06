/*
 * WS2801PixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 *
 *      This class is built as an adapter to control the WS2801 pixels with the RFPixel Control library.
 *      It is using a library which was written by Adafruit as the main pixel logic
 *      @see https://github.com/adafruit/Adafruit-WS2801-Library/ for more details
 *
 */

#ifndef WS2801PIXELCONTROL_H_
#define WS2801PIXELCONTROL_H_
#include <Adafruit_WS2801.h>
#include <IPixelControl.h>
#include <RFPixelControl.h>
class WS2801PixelControl : public Adafruit_WS2801, public IPixelControl{
public:
	WS2801PixelControl();
	 ~WS2801PixelControl();

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

#endif /* WS2801PIXELCONTROL_H_ */
