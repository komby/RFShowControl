/*
 * IPixelControl.h
 *
 *  Created on: Mar 19, 2013
 *      Author: Greg
 */

#ifndef IPIXELCONTROL_H_
#define IPIXELCONTROL_H_

#include <Arduino.h>


// Define RGB orderings
enum ColorOrder {
	RGB_ORDER=0012,
	RBG_ORDER=0021,
	GRB_ORDER=0102,
	GBR_ORDER=0120,
	BRG_ORDER=0201,
	BGR_ORDER=0210
};

//base class for pixels
class IPixelControl {
public:
	IPixelControl();
	
	
	virtual ~IPixelControl();


	/*
	 *  Begin so we can have some initalization
	 *
	 */
	void Begin(uint8_t* pDataPointer, int pNumLEDs);
 
	/*
	 * Return the protected value of the number
	 * of pixels this instance is configured for.
	 *
	 * @return the pixel count as a uint16_t
	 */
	uint16_t GetPixelCount( void );

	/*
	 * Sets a new value of pixels.  The function
	 * allocates the proper amount of memory to store
	 * each pixel's color data.
	 *
	 * @param count - Number of pixels in string
	 */
	void SetPixelCount( uint16_t count );

	/*
	 * Helper function to get the color of one pixel.
	 *
	 * @param n - Retrieve color for the nth pixel
	 *
	 * @return - uint32_t packed color of the nth pixel
	 */
	uint32_t GetPixelColor(uint16_t n);

	/*
	 * Sets pixel n to color c
	 *
	 * @param n - Pixel number to set
	 * @param c - Color to set the nth pixel to
	 */
	void SetPixelColor(uint16_t n, uint32_t c, uint8_t colorOrder = RGB_ORDER);
	/*
	 * Sets pixel n to values r, g, b
	 *
	 * @param n - Pixel number to set
	 * @param r - red 0-255 value
	 * @param g - green 0-255 value
	 * @param b - blue 0-255 value
	 */
	void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t colorOrder = RGB_ORDER);

	/*
	 * Create a 24 bit color value from R,G,B
	 *
	 * @param r - red 0-255 value
	 * @param g - green 0-255 value
	 * @param b - blue 0-255 value
	 *
	 * @return the color in a 24 bit pattern
	 */
	uint32_t Color(byte r, byte g, byte b);

	/*
	 * Pure virtual functions that must be implemented by all pixel
	 * type implementations.  This function loops and actually writes
	 * the color data stored in "pixels" to the LED string attached.
	 */
	void virtual Paint(void) = 0;
	
	/*
	 * Set the base data pointer as provided by 
	 * RFPixelControl and the OTAConfig
	 */
	void SetDataBasePointer( uint8_t* dataPointer);

protected:
	/*
	 * This stores our pixel data.  Pointer to a byte array that has
	 * numLEDs*3 worth of data.  One byte per "red", "green", and
	 * "blue" value is stored.
	 */
	uint8_t *pixels;

	/*
	 * Number of pixels we're configured for
	 */
	uint16_t numLEDs;

	/*
	 * Data pin we're configured to use to write to our pixels.
	 */
	uint8_t dataPin;
};

#endif // IPIXELCONTROL_H_
