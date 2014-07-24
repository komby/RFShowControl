/*
 * WM2999RFShowControl - Wrapper of the Adafruit WM2999 pixel library
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

#include "WM2999RFShowControl.h"

WM2999RFShowControl::WM2999RFShowControl(uint8_t ppin): WM2999(ppin)
{
  start();
}

void WM2999RFShowControl::Paint(void)
{
  printf("WM2999RFShowControl::Paint\n");
  this->WM2999::paint();
}

void WM2999RFShowControl::Begin(uint8_t* pDataPointer, int pNumLEDs)
{
  this->externalDataPointerSet = true;
  this->IRFShowControl::pixels = pDataPointer;
  this->WM2999::pixels = pDataPointer;
  this->numLEDs  = pNumLEDs;
  this->numberOfPixels = pNumLEDs;
}

void WM2999RFShowControl::SetDataBasePointer( uint8_t* dataPointer)
{
  this->IRFShowControl::pixels = dataPointer;
  this->WM2999::pixels = dataPointer;
  this->externalDataPointerSet = true;
}
