/*
 * WM2999PixelControl - Wrapper of the Adafruit WM2999 pixel library
 *
 * Created on: Mar 19, 2013
 * Author: Greg
 *
 * Updated: May 18, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *
 * License:
 *    Users of this software agree to hold harmless the creators and
 *    contributors of this software.  By using this software you agree that
 *    you are doing so at your own risk, you could kill yourself or someone
 *    else by using this software and/or modifying the factory controller.
 *    By using this software you are assuming all legal responsibility for
 *    the use of the software and any hardware it is used on.
 *
 *    The Commercial Use of this Software is Prohibited.
 */

#include "WM2999PixelControl.h"

WM2999PixelControl::WM2999PixelControl(uint8_t ppin): WM2999(ppin)
{
  start();
}

void WM2999PixelControl::Paint(void)
{
  printf("WM2999PixelControl::Paint\n");
  this->WM2999::paint();
}

void WM2999PixelControl::Begin(uint8_t* pDataPointer, int pNumLEDs)
{
  this->externalDataPointerSet = true;
  this->IPixelControl::pixels = pDataPointer;
  this->WM2999::pixels = pDataPointer;
  this->numLEDs  = pNumLEDs;
  this->numberOfPixels = pNumLEDs;
}

void WM2999PixelControl::SetDataBasePointer( uint8_t* dataPointer)
{
  this->IPixelControl::pixels = dataPointer;
  this->WM2999::pixels = dataPointer;
  this->externalDataPointerSet = true;
}
