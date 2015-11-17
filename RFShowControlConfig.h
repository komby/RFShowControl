/*
 * Author: Greg Scull/Mat Mrosko
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
#ifndef __RF_PIXEL_CONTROL_CONFIG__
#define __RF_PIXEL_CONTROL_CONFIG__

#include "RFShowControl.h"

#if (NRF_TYPE == MINIMALIST_SHIELD)
  #define    __CE  9
  #define    __CSN  8
  #elif (NRF_TYPE == MEGA_SHIELD)
  #define    __CE  46
  #define    __CSN  48
#elif \
  (NRF_TYPE == RF1) || \
  (NRF_TYPE == RF1_SERIAL) || \
  (NRF_TYPE == RF1_0_2) || \
  (NRF_TYPE == RF1_1_2) || \
  (NRF_TYPE == RF1_1_3) || \
  (NRF_TYPE == RF1_12V_0_1) ||\
  (NRF_TYPE == KOMBLINKIN) ||\
  (NRF_TYPE == KOMBEE)
    #define    __CE  8
    #define    __CSN  7
	#define HEARTBEAT_PIN  A1
	#define HEARTBEAT_PIN_1  3
	
#elif (NRF_TYPE == WM_2999_NRF) || \
  (NRF_TYPE == RFCOLOR_2_4) || \
  (NRF_TYPE == RAPTOR12) 
    #define    __CE  9
    #define    __CSN  10
#else
  #error Must define an NRF type!
#endif



#if (RF_WRAPPER==1)
RF24Wrapper radio =  RF24Wrapper(__CE, __CSN);
#else
RFShowControl radio =  RFShowControl(__CE, __CSN);
#endif


#if (PIXEL_TYPE == GWTS_EARS)
#include "GWTSControl.h"
#include <GwtS.h>
GWTSControl strip = GWTSControl();
#endif


#if (PIXEL_TYPE == GECE)

#if (HARDCODED_NUM_PIXELS > 63 )
  #error "GECE only uses 6 address bits, maximum of 63 pixels allowed!"
#endif

#include "GECERFShowControl.h"
#include <GEColorEffects.h>
GECERFShowControl strip = GECERFShowControl(PIXEL_DATA_PIN, HARDCODED_NUM_PIXELS, LOGICAL_DATA_ORDER_1);
#warning "PIXEL_COLOR_ORDER Unsupported for GECEs"
#endif


#if (PIXEL_TYPE == RENARD)

#if ((RENARD_BAUD_RATE != 460800) && \
     (RENARD_BAUD_RATE != 230400) && \
     (RENARD_BAUD_RATE != 115200) && \
     (RENARD_BAUD_RATE != 57600) && \
     (RENARD_BAUD_RATE != 38400) && \
     (RENARD_BAUD_RATE != 19200))
  #error "Invalid Renard BAUD RATE"
#endif

#if (RENARD_BAUD_RATE == 460800)
#if (HARDCODED_NUM_CHANNELS >= 4584)
  #error "Cannot have more than 4584 channels with 460800 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 2292)
  #warning "Cannot have more than 2292 channels with 460800 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 1146)
  #warning "Cannot have more than 1146 channels with 460800 baud with 25ms refresh interval"
#endif
#endif
#endif

#if (RENARD_BAUD_RATE == 230400)
#if (HARDCODED_NUM_CHANNELS >= 2292)
  #error "Cannot have more than 2292 channels with 230400 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 1146)
  #warning "Cannot have more than 1146 channels with 230400 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 573)
  #warning "Cannot have more than 573 channels with 230400 baud with 25ms refresh interval"
#endif
#endif
#endif

#if (RENARD_BAUD_RATE == 115200)
#if (HARDCODED_NUM_CHANNELS >= 1146)
  #error "Cannot have more than 1146 channels with 115200 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 573)
  #warning "Cannot have more than 573 channels with 115200 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 285)
  #warning "Cannot have more than 285 channels with 115200 baud with 25ms refresh interval"
#endif
#endif
#endif

#if (RENARD_BAUD_RATE == 57600)
#if (HARDCODED_NUM_CHANNELS >= 573)
  #error "Cannot have more than 573 channels with 57600 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 285)
  #warning "Cannot have more than 285 channels with 57600 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 141)
  #warning "Cannot have more than 141 channels with 57600 baud with 25ms refresh interval"
#endif
#endif
#endif

#if (RENARD_BAUD_RATE == 38400)
#if (HARDCODED_NUM_CHANNELS >= 381)
  #error "Cannot have more than 381 channels with 38400 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 189)
  #warning "Cannot have more than 189 channels with 38400 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 93)
  #warning "Cannot have more than 93 channels with 38400 baud with 25ms refresh interval"
#endif
#endif
#endif

#if (RENARD_BAUD_RATE == 19200)
#if (HARDCODED_NUM_CHANNELS >= 189)
  #error "Cannot have more than 189 channels with 19200 baud, even with 100ms refresh interval"
#endif
#if (HARDCODED_NUM_CHANNELS >= 93)
  #warning "Cannot have more than 93 channels with 19200 baud with 50ms refresh interval"
#if (HARDCODED_NUM_CHANNELS >= 45)
  #warning "Cannot have more than 45 channels with 19200 baud with 25ms refresh interval"
#endif
#endif
#endif
#endif


#if (PIXEL_TYPE == WM_2999)
#include <WM2999.h>
#include "WM2999RFShowControl.h"
WM2999RFShowControl strip = WM2999RFShowControl(PIXEL_DATA_PIN_1);
#warning "PIXEL_COLOR_ORDER Unsupported for WM2999"
#endif


#if (PIXEL_TYPE == LPD_6803)
#ifndef ADAFRUITILIB6803
#define ADAFRUITILIB6803
#include <LPD6803.h>
#endif
#include "LPD6803RFShowControl.h"
#include <TimerOne.h>
LPD6803RFShowControl strip = LPD6803RFShowControl(PIXEL_DATA_PIN, PIXEL_CLOCK_PIN_1);
#warning "PIXEL_COLOR_ORDER Unsupported for LPD6803"
#endif

#if (PIXEL_TYPE == STROBE)
#include <ShiftPWM.h>
#include <lib8tion.h>
#ifndef ShiftPWM_H
#error "You are missing the ShiftPWM library you need to download it from 'https://github.com/elcojacobs/ShiftPWM' and put it in your libraries folder"
#endif
#ifndef __INC_LIB8TION_H
#error "You are missing the FastLED Library.  You need to download it from 'https://github.com/FastLED/FastLED' and put it in your libraries folder"
#endif


#endif


#ifdef PIXEL_TYPE
#if ((PIXEL_TYPE == LPD_8806) || \
   (PIXEL_TYPE == WS_2801) || \
   (PIXEL_TYPE == SM_16716) || \
   (PIXEL_TYPE == TM_1809) || \
   (PIXEL_TYPE == TM_1803) || \
   (PIXEL_TYPE == UCS_1903) || \
   (PIXEL_TYPE == WS_2812B) || \
    (PIXEL_TYPE == NEO_PIXEL) || \
	 (PIXEL_TYPE == TM_1829) || \
	  (PIXEL_TYPE == LPD_1886) || \
	   (PIXEL_TYPE == APA_104) || \
   (PIXEL_TYPE == WS_2811))
#include <FastLED.h>
CRGB *leds;
#define FAST_SPI_CONTROL
#ifndef __INC_FASTSPI_LED2_H
#error "You are missing the newest FastLED Library.  You need to download it from 'https://github.com/FastLED/FastLED' and put it in your libraries folder"
#endif
#endif
#endif


// Radio pipe
// addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};


#if ((PIXEL_TYPE != GECE) && \
    (PIXEL_TYPE != WS_2801) && \
    (PIXEL_TYPE != WS_2811) && \
	(PIXEL_TYPE != WS_2812B) && \
    (PIXEL_TYPE != RENARD) && \
    (PIXEL_TYPE != WM_2999) && \
    (PIXEL_TYPE != LPD_6803) && \
    (PIXEL_TYPE != GWTS_EARS) && \
    (PIXEL_TYPE != LPD_8806) && \
    (PIXEL_TYPE != SM_16716) && \
    (PIXEL_TYPE != TM_1809) && \
    (PIXEL_TYPE != TM_1803) && \
    (PIXEL_TYPE != UCS_1903) && \
    (PIXEL_TYPE != DMX) && \
	(PIXEL_TYPE != NEO_PIXEL) && \
	(PIXEL_TYPE != TM_1829) && \
	(PIXEL_TYPE != LPD_1886) && \
	(PIXEL_TYPE != APA_104) && \
	(PIXEL_TYPE != DMX) && \
	(PIXEL_TYPE != STROBE) && \
    (PIXEL_TYPE != NONE))
  #error "You must define PIXEL_TYPE as one of the following:"
  #error "  GECE, WS_2801, WS_2811, RENARD, WM_2999,LPD_6803 , GWTS_EARS"
  #error "  LPD_8806, SM_16716, TM_1809, TM_1803, UCS_1903, DMX, STROBE"
#endif

#if (FCC_RESTRICT && LISTEN_CHANNEL > 83 && LISTEN_CHANNEL < 101 )
  #error "FCC Restricts the use of channels 84-100 see http://learn.komby.com/wiki/58/configuration-settings#FCC_RESTRICT"
#endif

#endif //__RF_PIXEL_CONTROL_CONFIG__
