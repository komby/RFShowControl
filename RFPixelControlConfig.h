#ifndef __RF_PIXEL_CONTROL_CONFIG__
#define __RF_PIXEL_CONTROL_CONFIG__


#if (NRF_TYPE == MINIMALIST_SHIELD)
RFPixelControl radio(9, 8);
#else
#if (NRF_TYPE == RF1_1_2) || \
	(NRF_TYPE == RF1_1_3) || \
	(NRF_TYPE == RF1_0_2) || \
	(NRF_TYPE == RF1_12V_0_1)
RFPixelControl radio(8, 7);
#else
#if (NRF_TYPE == KOMBYONE_DUE)
RFPixelControl radio(33,10);
#else
#error Must define an NRF type!
#endif
#endif
#endif







#if (PIXEL_TYPE == GECE)
#include "GECEPixelControl.h"
#include <GEColorEffects.h>
GECEPixelControl strip = GECEPixelControl(PIXEL_PIN);
#endif


#if (PIXEL_TYPE == RENARD)
#include "RenardControl.h"
RenardControl strip = RenardControl(RENARD_BAUD_RATE);
#endif





// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
	0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};


#endif //__RF_PIXEL_CONTROL_CONFIG__
