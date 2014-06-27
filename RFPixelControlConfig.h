/*
 * Author: Greg Scull/Mat Mrosko
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
#ifndef __RF_PIXEL_CONTROL_CONFIG__
#define __RF_PIXEL_CONTROL_CONFIG__

#include "RFPixelControl.h"

#if (NRF_TYPE == MINIMALIST_SHIELD)
	#define		__CE	9
	#define		__CSN	8
#elif \
	(NRF_TYPE == RF1) || \
	(NRF_TYPE == RF1_0_2) || \
	(NRF_TYPE == RF1_1_2) || \
	(NRF_TYPE == RF1_1_3) || \
	(NRF_TYPE == RF1_12V_0_1) ||\
	(NRF_TYPE == KOMBEE)
		#define		__CE	8
		#define		__CSN	7
#elif (NRF_TYPE == WM_2999_NRF) || \
	(NRF_TYPE == RFCOLOR_2_4)
		#define		__CE	9
		#define		__CSN	10
#else
	#error Must define an NRF type!
#endif


#if (RF_WRAPPER==1)
RF24Wrapper radio(__CE, __CSN);
#else
RFPixelControl radio(__CE, __CSN);
#endif


#if (PIXEL_TYPE == GWTS_EARS)
#include "GWTSControl.h"
#include <GwtS.h>
GWTSControl strip = GWTSControl();
#endif


#if (PIXEL_TYPE == GECE)
#include "GECEPixelControl.h"
#include <GEColorEffects.h>
GECEPixelControl strip = GECEPixelControl(PIXEL_DATA_PIN, HARDCODED_NUM_PIXELS);
#endif


#if (PIXEL_TYPE == RENARD)
#include "RenardControl.h"
RenardControl strip = RenardControl(RENARD_BAUD_RATE);
#endif


#if (PIXEL_TYPE == WM_2999)
#include <WM2999.h>
#include "WM2999PixelControl.h"
WM2999PixelControl strip = WM2999PixelControl(PIXEL_DATA_PIN);
#endif


#if (PIXEL_TYPE == LPD_6803)
#ifndef ADAFRUITILIB6803
#define ADAFRUITILIB6803
#include <LPD6803.h>
#endif
#include "LPD6803PixelControl.h"
#include <TimerOne.h>
LPD6803PixelControl strip = LPD6803PixelControl(PIXEL_DATA_PIN, PIXEL_CLOCK_PIN);
#endif





#ifdef PIXEL_TYPE
#if ((PIXEL_TYPE == LPD_8806) || \
	 (PIXEL_TYPE == WS_2801) || \
	 (PIXEL_TYPE == SM_16716) || \
	 (PIXEL_TYPE == TM_1809) || \
	 (PIXEL_TYPE == TM_1803) || \
	 (PIXEL_TYPE == UCS_1903) || \
	 (PIXEL_TYPE == WS_2811))
#include <FastLED.h>
CRGB *leds;
#define FAST_SPI_CONTROL
#endif
#endif




// Radio pipe
// addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
	0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};


#endif //__RF_PIXEL_CONTROL_CONFIG__
