/*
 * GWTSControl - Wrapper of the GwtS library for IRFShowControl
 *
 * Created on: November 25th, 2013
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

#include "GWTSControl.h"

GWTSControl::GWTSControl(void) : GWTS()
{
}


/**
 * Paint method to keep compatibility with the other Pixel types
 *
 */
void GWTSControl::Paint(void)
{
  this->set_colors(pixels[0],
                   pixels[1],
                   pixels[2],
                   pixels[3],
                   pixels[4],
                   pixels[5]);
}
