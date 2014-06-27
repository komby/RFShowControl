/*
 * FastSPI_LED2_PixelControl - This class is built as an adapter to control all of the pixels supported by FastSPI_LED2 with the RFPixel Control library.
 *
 * Created on: May 28th, 2014
 * Author: Mat Mrosko
 *
 * Updated: June 25th 2014 Greg Scull - Renaming to reflect the Fast SPI Library rename
 *
 * License:
 *		Users of this software agree to hold harmless the creators and
 *		contributors of this software.  By using this software you agree that
 *		you are doing so at your own risk, you could kill yourself or someone
 *		else by using this software and/or modifying the factory controller.
 *		By using this software you are assuming all legal responsibility for
 *		the use of the software and any hardware it is used on.
 *
 *		The Commercial Use of this Software is Prohibited.
 */

#include "FastLED_PixelControl.h"

FastLED_PixelControl::FastLED_PixelControl(CLEDController *pLed) : mLed(pLed)
{
}

void FastLED_PixelControl::Begin(RFPixelControl *radio)
{
	LEDS.setBrightness(128);
	leds = (CRGB*) radio->GetControllerDataBase(0);
	int countOfPixels = radio->GetNumberOfChannels(0)/3;
	memset(leds, 0, countOfPixels * sizeof(struct CRGB));
	LEDS.addLeds(mLed, leds, countOfPixels, 0);
}

/**
 * Paint method to keep compatibility with the other Pixel types
 */
void FastLED_PixelControl::Paint(void)
{
	LEDS.show();
}
