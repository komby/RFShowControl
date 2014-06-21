/*
SCR driver for the Raptor12
 
 Adapted from sketch by by Robert Twomey <rtwomey@u.washington.edu>
 Adapted from sketch by Ryan McLaughlin <ryanjmclaughlin@gmail.com> 
 */



#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <TimerOne.h>  

/*************************** CONFIGURATION SECTION *************************************************/
// Define a Unique receiver ID.  This id should be unique for each receiver in your setup. 
// If you are not using Over The air Configuration you do not need to change this setting.
//Valid Values: 1-255
#define RECEIVER_UNIQUE_ID 33

 //What board are you using to connect your nRF24L01+?
 //Valid Values:  MINIMALIST_SHIELD, RF1_1_2, RF1_1_3, RF1_0_2, RF1_12V_0_1, KOMBYONE_DUE, WM_2999_NRF, RFCOLOR2_4
 #define NRF_TYPE  WM_2999_NRF

// Set OVER_THE_AIR_CONFIG_ENABLEG to 1 if you are making a configuration node to re-program
// your RF1s in the field.  This will cause the RF1s to search for a
// configuration broadcast for a short period after power-on before attempting to
// read EEPROM for the last known working configuration.
#define OVER_THE_AIR_CONFIG_ENABLE 0

#define NUM_CHANNELS 12 //You should not relly ever need to change this.  
#define START_CHANNEL 1 //Where in the universe do we start
#define FINAL_CHANNEL 12 //DO Refactory out, addition would be easy.... 


//What RF Channel do you want to listen on?  
//Valid Values: 1-124
#define LISTEN_CHANNEL 100	

//What Speed is your transmitter using?
//Valid Values   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//IS this an AC controler?
#define acControler true

 /**************END CONFIGURATION SECTION ***************************/
//Include this after all configuration variables are set
#include <RFPixelControlConfig.h>
#define RECEIVER_NODE 1
bool readytoupdate=false;

byte * buffer;


//Uncomment for serial
#define DEBUG 0



 
 int totalChannel = 12;
 ///0-127 aka 7bits
 //int channelLevel [] = {20,20,30,40,50,60,70,90,100,110,120,127}; //dim level 0-255 just for testing
 
  volatile int dimLevel=255;               // Variable to use as a counter
  volatile boolean zero_cross = false;  // Boolean to store a "switch" to tell us if we have crossed zero
  int lastLevel;                         //This is for the Triac fireing function to do a bit of math


void setup() {  // Begin setup  
    pinMode(2, INPUT);
      attachInterrupt(0, zeroCrossDetect, CHANGE );      // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  int freqStep = 32;                                     // Just guess and check at the moment  | 1000000 uS / 60 Hz) / 256 brightness steps = 16.25
  Timer1.initialize(freqStep);                           // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(zeroCrossEvent, freqStep);      // See if we can fire the SCR.
   
  //Setup output port pins
  // B0    C0 C1 C2 C3 C4 C5    D3 D4 D5 D6 D7
  //useing OR to make sure I dont modify any other pins.
    DDRB = DDRB | B00000001; 
    DDRC = DDRC | B00111111;
    DDRD = DDRD | B11111000;
    
    
   //nrf komby stuff
   	Serial.begin(57600);
 	buffer[0]=255;
   
    
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
         int logicalControllerSequenceNum = 0;
         radio.AddLogicalController(logicalControllerSequenceNum, START_CHANNEL, NUM_CHANNELS,0);
	}
	//printf_begin();//Probbly shoudl go figureout that printF incude
	
   	delay(2);

     radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
      radio.printDetails(); 
      //initalize data buffer
      buffer= radio.GetControllerDataBase(0);
	delay (200); //needed or leftover code?



}

void zeroCrossDetect() { 
  if (dimLevel > 220){ //DO not triger again untill were at lowest dim level with a bit of slop
    dimLevel=0;   
    zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured 

    //Turn off all scrs here just in case should be off already?
    PORTB =  PORTB & B11111110; 
    PORTC =  PORTC & B11000000;
    PORTD =  PORTD & B00000111;
  }
} 
//When ever one of the diming points are reached run this to enable pins
void zeroCrossEvent() {                   
  if(zero_cross == true) {   
    
int invDimLevel = dimLevel ^ 255;// 255 accely means off in this loop so we need to invert the value
////////////////////////////////////////////////////////////////////////////////////////
//////////////////note to self make this a function ////////////////////////////////////
///Channel 1
if (buffer[0] == invDimLevel){
  PORTB = PORTB | B00000001; 
}//Channel 2//////////////////////////////PortC
if (buffer[1] == invDimLevel){
  PORTC =  PORTC | B00100000; 
}//Channel 3
if (buffer[2] == invDimLevel){
  PORTC =  PORTC | B00010000;
}//Channel 4
if (buffer[3] == invDimLevel){
  PORTC =  PORTC | B00001000;
}//Channel 5
if (buffer[4] == invDimLevel){
  PORTC =  PORTC | B00000100;
}//Channel 6
if (buffer[1] == invDimLevel){
  PORTC =  PORTC | B00000010;
}//Channel 7
if (buffer[6] == invDimLevel){
  PORTC =  PORTC | B00000001;
}//Channel 8/////////////////////////////////PortD
if (buffer[7] == invDimLevel){
  PORTD =  PORTD | B10000000;
}//Channel 9
if (buffer[8] == invDimLevel){
  PORTD =  PORTD | B01000000; 
}//Channel 10
if (buffer[9] == invDimLevel){
  PORTD =  PORTD | B00100000; 
}//Channel 11
if (buffer[10] == invDimLevel){
  PORTD =  PORTD | B00010000; 
}//Channel 12
if (buffer[11] == invDimLevel){
  PORTD =  PORTD | B00001000; 
}

///unused pins 
//}//Channel 13  //Normaly ZeroCross
//if (buffer[12] == invDimLevel){
//  PORTD =  PORTD | B00000100; 
//}
//}//Channel 14  //Normaly TX
//if (buffer[13] == invDimLevel){
//  PORTD =  PORTD | B00000010; 
//} 
//}//Channel 15  //Normaly RX
//if (buffer[14] == invDimLevel){
//  PORTD =  PORTD | B00000001; 
}
//////////////////////////////////////////////////////////////////////////////////////////
    dimLevel = dimLevel + 1; // decrese time step counter                     
    }                                                                 
}

void loop() {
radio.Listen();

//If we donet have to worry about zerocross just pretend we got a zerocross
if (acControler == false){
  if (dimLevel >= 255){
    zeroCrossDetect();
    }
  }
}
