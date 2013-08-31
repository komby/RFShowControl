#ifndef __RF_PIXEL_CONTROL_CONFIG__
#define __RF_PIXEL_CONTROL_CONFIG__


#if (NRF_TYPE == MINIMALIST_SHIELD)
RFPixelControl radio(9, 8);
#elif (NRF_TYPE == RF1_1_2) || \
	(NRF_TYPE == RF1_1_3) || \
	(NRF_TYPE == RF1_0_2) || \
	(NRF_TYPE == RF1_12V_0_1) ||\
	(NRF_TYPE == KOMBEE) 
RFPixelControl radio(8, 7);
#elif (NRF_TYPE == KOMBYONE_DUE)
RFPixelControl radio(33,10);
#elif (NRF_TYPE == WM_2999_NRF) || \
	(NRF_TYPE == RFCOLOR2_4)
RFPixelControl radio(9,10);
#else
#error Must define an NRF type!
#endif







#if (PIXEL_TYPE == GECE)
#include "GECEPixelControl.h"
#include <GEColorEffects.h>
GECEPixelControl strip = GECEPixelControl(PIXEL_DATA_PIN);
#endif


#if (PIXEL_TYPE == RENARD)
#include "RenardControl.h"
RenardControl strip = RenardControl(RENARD_BAUD_RATE);
#endif

#if (PIXEL_TYPE == WM_2999)
#include <WM2999.h>
#include "WM2999PixelControl.h"
WM2999PixelControl strip =  WM2999PixelControl(PIXEL_DATA_PIN);
#endif

#if (PIXEL_TYPE == LPD_6803)
#ifndef ADAFRUITILIB6803
#define ADAFRUITILIB6803
#include <LPD6803.h>
#endif
#include "LPD6803PixelControl.h"
#include <TimerOne.h>
LPD6803PixelControl  strip =  LPD6803PixelControl(PIXEL_DATA_PIN, PIXEL_CLOCK_PIN);
#endif


// Radio pipe
// addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
	0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};


#endif //__RF_PIXEL_CONTROL_CONFIG__
