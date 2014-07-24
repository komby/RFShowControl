/*

 SoftPWM SCR driver for the Raptor12
 
 SoftPWM is an all in one PWM driver for 12 channels based on a Atmega328 chipset.  It supports
 zerocross detection as well as simulated ZC for use as a DC based controller as well as an NON-Isolated
 AC based contoroller.
 
 Created by Travis Kneale   2014
 Adapted from sketch by by Robert Twomey <rtwomey@u.washington.edu>
 Adapted from sketch by Ryan McLaughlin <ryanjmclaughlin@gmail.com> 
 
 Modified 7/9/2014 Greg Scull - komby@komby.com to work with Clarity release 
 
 
 */

#include <Arduino.h>
#include <RFShowControl.h>
#include <IRFShowControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <TimerOne.h>


/*************************** CONFIGURATION SECTION *************************************************/
// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/wiki/58/configuration-settings#RECEIVER_UNIQUE_ID
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/wiki/58/configuration-settings#OVER_THE_AIR_CONFIG_ENABLE
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RAPTOR12
#define NRF_TYPE  RAPTOR12

// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#AC_DC
// Valid Values: 1, 0
#define AC_DC  1


/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 0-83, 101-127  (Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_START_CHANNEL
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL         1

// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_CHANNELS
// Valid Values: 1
// strobe control only uses one channel
#define HARDCODED_NUM_CHANNELS          12

/******************* END OF NON-OTA CONFIGURATION SECTION ********************/


/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
#define PIXEL_TYPE NONE





#define FINAL_CHANNEL 12 //DO Refactor out, addition would be easy.... 


//IS this an AC controller?
bool acControler = AC_DC;
 
// Just guess and check at the moment  
// 1000000 uS / 120 Hz ) / (256 + 60) = 26(32old)brightness steps = 16.25
#define freqStep  26


 /**************END CONFIGURATION SECTION ***************************/
//Include this after all configuration variables are set
#define RECEIVER_NODE 1
#include <RFPixelControlConfig.h>

bool readytoupdate=false;

byte * buffer;
byte * bufferOutput;

//Uncomment for serial
#define DEBUG 0

 
 int totalChannel = 12;
 ///0-127 aka 7bits
 //int channelLevel [] = {20,20,30,40,50,60,70,90,100,110,120,127}; //dim level 0-255 just for testing
 
  volatile int dimLevel=316;               // Variable to use as a counter
  volatile boolean zero_cross = false;  // Boolean to store a "switch" to tell us if we have crossed zero
  int lastLevel;                         //This is for the Triac firing function to do a bit of math


void setup() {  // Begin setup  
    pinMode(2, INPUT);
      attachInterrupt(0, zeroCrossDetect, HIGH );      // Attach an Interrupt to Pin 2 (interrupt 0) for Zero Cross Detection
  
  Timer1.initialize(freqStep);                           // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(zeroCrossEvent, freqStep);      // See if we can fire the SCR.
   
  //Setup output port pins
  // B0    C0 C1 C2 C3 C4 C5    D3 D4 D5 D6 D7
  //using OR to make only the correct pins are modified.
    DDRB = DDRB | B00000001; 
    DDRC = DDRC | B00111111;
    DDRD = DDRD | B11111000;
    
 
   	Serial.begin(115200);
 	buffer[0]=255;
   
    
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
         int logicalControllerSequenceNum = 0;
         radio.AddLogicalController(logicalControllerSequenceNum, HARDCODED_START_CHANNEL, HARDCODED_NUM_CHANNELS,0);
	}
	
   	delay(2);

     radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
      radio.printDetails(); 
    
	//initialize data buffer
    buffer= radio.GetControllerDataBase(0);
	
	delay (200); //needed or leftover code?



}


void triggerOutputPin(){
  noInterrupts();
  int dimOffsetLevel = dimLevel - 30;
  interrupts();
    if (dimLevel <= 30){   
    //PORTB =  PORTB & B11111110; 
   // PORTC =  PORTC & B11000000;
   // PORTD =  PORTD & B00000111;
  } 
  
  if (dimOffsetLevel <= 255){
    if(dimOffsetLevel >= 1){ 
      ///Channel 1
      if (buffer[0] >= dimOffsetLevel ){
        PORTB = PORTB | B00000001; 
      }//Channel 2
      if (buffer[1] >= dimOffsetLevel){
        PORTC =  PORTC | B00000001;
      }//Channel 3
      if (buffer[2] >= dimOffsetLevel){
        PORTC =  PORTC | B00000010;
      }//Channel 4
      if (buffer[3] >= dimOffsetLevel){
        PORTC =  PORTC | B00000100;
      }//Channel 5
      if (buffer[4] >= dimOffsetLevel){      
        PORTC =  PORTC | B00001000;
      }//Channel 6
      if (buffer[5] >= dimOffsetLevel){        
        PORTC =  PORTC | B00010000;
      }//Channel 7
      if (buffer[6] >= dimOffsetLevel){        
        PORTC =  PORTC | B00100000;        
      }//Channel 8
      if (buffer[7] >= dimOffsetLevel){
        PORTD =  PORTD | B00001000; 
      }//Channel 9
      if (buffer[8] >= dimOffsetLevel){
        PORTD =  PORTD | B00010000;
      }//Channel 10
      if (buffer[9] >= dimOffsetLevel){
        PORTD =  PORTD | B00100000; 
      }//Channel 11
      if (buffer[10] >= dimOffsetLevel){
        PORTD =  PORTD | B01000000; 
      }//Channel 12
      if (buffer[11] >= dimOffsetLevel){
        PORTD =  PORTD | B10000000;
      }
    }   
  }
}

void zeroCrossDetect() {
  if (dimLevel <= 4
  00){
  noInterrupts();   
  dimLevel=316;
  PORTB =  PORTB & B11111110; 
  PORTC =  PORTC & B11000000;
  PORTD =  PORTD & B00000111; 
  //this may break timer1 in the future
  TCNT1 = 0; // Reset timer value attempt to stop it overfilling  
  interrupts();
  }
} 

//When ever one of the dimming points are reached run this to enable pins
void zeroCrossEvent() {  
  if (dimLevel > 0){ //this will prevent this var going negative
    dimLevel = dimLevel - 1; // decrease time step counter 
    
  }else{
    //if we missed the zero cross fake where it should be
    if (lastLevel >= 30){ 
     zeroCrossDetect();
      dimLevel = dimLevel - lastLevel;
      lastLevel = 0;
    }
    lastLevel = lastLevel + 1;
  }
  //38khz

triggerOutputPin(); 

}                                                                 






void loop() {
 // PORTB = PORTB | B00000001;
  if(radio.Listen()){
    bufferOutput = buffer;
   }
 // PORTB = PORTB & B11111110;
 // while(true){
    //18khz //10khz without while
 //PORTB = PORTB | B00000001;
 //triggerOutputPin();
// PORTB = PORTB & B11111110;
 // if (dimLevel == 20){ 
  //    
  
   //PORTB = PORTB & B11111110;
 // }
  
}
