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


class WM2999PixelControl : public WM2999, public IPixelControl {
public:

        WM2999PixelControl(uint8_t ppin): WM2999(ppin){
         
        }
	 ~WM2999PixelControl(void){

          }

	void Paint(uint8_t * colors , unsigned int count){
                    paint(colors,count);
                }
		void  Paint(void){
                    paint();
                }
		uint32_t  GetPixelCount( void ){
                    return getPixelCount();
                }
		void  SetPixelCount( uint32_t pCount){
                 setPixelCount(pCount);
                }
		void  Start( void ){
                  start();
                }
		uint32_t  GetPixelColor(uint16_t n){
                  return getPixelColor(n);
                }

		void  SetPixelColor(uint16_t n, uint32_t c){
                 setPixelColor(n, c);
                }
		void  SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b){
                    setPixelColor(n, r, g, b);
                }
                uint32_t Color(byte r, byte g, byte b){
                   return color(r, g, b); 
                }
                void ColorWipe(uint32_t c, uint8_t wait){
                   colorWipe(c, wait); 
                }
};

#endif /* WM2999PIXELCONTROL_H_ */



