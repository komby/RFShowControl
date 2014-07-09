/*
 * RenardControl
 *
 * Created on: June 20th, 2013
 * Author: Mat Mrosko
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

#include "RenardControl.h"

RenardControl::RenardControl(uint32_t baud_rate)
{
  Serial.begin(baud_rate);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
}


void RenardControl::Paint(void)
{
  Serial.write(0x7E);
  Serial.write(0x80);

  for ( int i = 0; i < (numLEDs); i++ )
  {
    switch (pixels[i])
    {
      case 0x7D:
        Serial.write(0x7F);
        Serial.write(0x2F);
        break;

      case 0x7E:
        Serial.write(0x7F);
        Serial.write(0x30);
        break;

      case 0x7F:
        Serial.write(0x7F);
        Serial.write(0x31);
        break;

      default:
        Serial.write(pixels[i]);
        break;
    }
  }
}
