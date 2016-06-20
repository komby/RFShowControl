/*
 RF_In_SSR_Out   Triac driver for the Komby RF1 SSR4 Expansion Board and Komby SSR8
 
 RF_In_SSR_Out is a PWM driver for up to 8 channels based on a Atmega328 chipset.  
 This Sketch must be used with hardware zero cross detection.
 The Pin designations for the radio and outputs are specific to a Komby RF1 and the SSR8
 The code should run fine with some modifications on most atmega chips.
 Please see http://learn.komby.com/ for information on the configuration variables
 
 The Sketch detects the zero crossing of the AC sine wave from a pulse on pin 3 which is provided by
 the circuitry on the Triac Expansion board or the RF1 SSR8 controller. We need this pulse to know when to start a timer that decrements
 a fixed amount of microseconds each period. This is used to convert the DMX values (0 - 255) provided by the rfShowControl library
 into a time delay. This time delay is used to turn the Triacs on. A small DMX value will result in a long time delay.
 A Large DMX value will result in a shorter delay. The Longer time delay in turning on the triac, the dimmer the lights 
 connected to it will be. Once a Triac is turned on, it will remain latched on, even if the gate voltage is removed,
 until the input voltage crosses zero. We turn the triac off before it reaches the next zero crossing,
 otherwise it will remain on. This sketch uses interrupts both on an input pin for zero crossing and on a timer
 to set the time periods in which to check the dimming levels and see if it's time to turn the triac on.
 
 Tim Benson 2016
 Adapted from sketch by by Travis Kneale   2014
 Adapted from sketch by by Robert Twomey <rtwomey@u.washington.edu>
 Adapted from sketch by Ryan McLaughlin <ryanjmclaughlin@gmail.com> 
 
 */

#include <Arduino.h>
#include <RFShowControl.h>
#include <IRFShowControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <TimerOne.h>
#include <printf.h>

/********************************************** CONFIGURATION SECTION *********************************************************/
// NRF_TYPE    What type of Board are we programming ?   Valid Value: RF1
#define NRF_TYPE                        RF1  

// RECEIVER_UNIQUE_ID  You Only Need to change this if you are using Over the Air configuration.  Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33

// OVER_THE_AIR_CONFIG_ENABLE   Did you set up a Configuration Transmitter ?   Valid Values: 0 = Not Enabled   1 = Enabled
#define OVER_THE_AIR_CONFIG_ENABLE      0

// LISTEN_CHANNEL  Which NRF channel will be be listening on ?   Valid Values: 0-83, 101-127  
//(Note: use of channels 84-100 is not allowed in the US)
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL  Which DMX Channel is the first output channel of this device ?  Valid Values: 1-509
#define HARDCODED_START_CHANNEL         1

// HARDCODED_NUM_CHANNELS  The Triac Expansion Board supports 1 to 4 channels. The SSR8 supports 1 to 8 channels.   Valid Values: 1-8
#define HARDCODED_NUM_CHANNELS          4

// AC_FREQUENCY_50  If your AC Frequency is 50HZ, you need to uncomment the next line
//#define AC_FREQUENCY_50

// If you want to enable Verbose Serial port output, Uncomment the next line. This should only be used for debugging purposes
//#define DEBUG 1

/****************************************** END CONFIGURATION SECTION *********************************************************/
/************************************* START OF ADVANCED SETTINGS SECTION (OPTIONAL) ******************************************/

// No Pixels used here, Don't change this
#define PIXEL_TYPE NONE  

//FCC_RESTRICT   Non US Users can use restricted bands        Valid Values: 1, 0  
//(1 will prevent the use of channels that are not allowed in North America)
#define FCC_RESTRICT 1

// This is the delay-per-brightness step in microseconds and how many steps we will have in each half sine wave.
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want.
// Don't mess with this unless you know what you are doing as this will affect the trigger points for the Triacs
// 50 Hz Calculations
// 1000000 us / (50 Hz * 2) = 10000 uS, length of one half-wave.
// 8333 uS / 128 steps = 78 uS / step
// 8333 uS / 256 steps = 39 uS / step
// 8333 uS / 316 steps = 31 uS / step    256 + 60  Offset both sides of the dimming cycle with 30 steps  (930 microseconds)
// 60 Hz Calculations
// 1000000 us / (60 Hz * 2) =  8333 uS, length of one half-wave.
// 8333 uS / 128 steps = 65 uS / step
// 8333 uS / 256 steps = 32 uS / step
// 8333 uS / 316 steps = 26 uS / step    256 + 60  Offset both sides of the dimming cycle with 30 steps  (780 microseconds)
//
#define dmxSteps     256   //The DMX data has 256 possible values
#define offsetSteps  30   //We will have 30 steps offset on each end of the dimming cycle
#define numSteps     316  //We will have 316 steps per dimming cycle
#define timeStep     26   //We will make each step 26 microseconds
// If we have 50 Hz. we need to change the timeStep
#ifdef  AC_FREQUENCY_50 
  #define timeStep   31   //We will make each step 31 microseconds
#endif

/************************************* END OF ADVANCED SETTINGS SECTION (OPTIONAL) ********************************************/
/******************************************************** Begin Global Variables ******************************************************************/

// Include this after all configuration variables are set
#include "RFShowControlConfig.h"   // Load up the RFShowControl Header file
// buffer[] is where the output of the RFShowControl Datastream will be put. It's part of the "Komby Magic"
byte * buffer;                     // Pointer, which declares the data stored at memory location "buffer" should be treated as bytes
byte * bufferOutput;               // Pointer, which declares the data stored at memory location "bufferOutput" should be treated as bytes
volatile int dimLevel = numSteps;  // Variable to use as a counter to track the dimming point , Read from RAM,   initialize to value of numSteps
int dataLedCycle = 120;            // set a 1 sec blink cycle
int dataAvaliable = 120;           // Variable to tell us if channel data is available

/******************************************************** End of Global Variables ******************************************************************/
/************************************* Begin setup ,  setup runs once then we enter into the main loop. ********************************************/

void setup() {   
// Set up the Zero Crossing Detection
pinMode(3, INPUT);                                 // Set up Pin D3 for input
attachInterrupt(1, zeroCrossDetect, RISING );      // Attach an Interrupt to Pin 3 for RF1 (interrupt 1) for Zero Cross Detection

// Set up the timer for the dimming points and the interrupt that will call the routine dimPointCheck                              
Timer1.initialize(timeStep);                       // Initialize TimerOne library for the time step we need
Timer1.attachInterrupt(dimPointCheck, timeStep);   // run dimPointCheck every timeStep
   
// Setup output port pins
// Port Pins  B1 B2  D5 D6 C1 C2 C3 C4 , Arduino Pins  5,6,9,10,A1,A2,A3,A4
// We are writing directly to the output pins so make sure to define only the pins we use for Triacs
// using OR "|" to make sure only the correct pins are modified.
DDRB = DDRB | B00000110;    // Sets Pins 1 and 2 of port B as outputs, leaving other pins alone by way of the or function
DDRD = DDRD | B01100000;    // Sets Pins 5 and 6 of port D as outputs, leaving other pins alone by way of the or function
DDRC = DDRC | B00011111;    // Set Pins 1 2 3 4 of port C as outputs, Set Pin 0 (A0) up for Radio Status LED,  (A RF1 only has A0 connected)
  
// If we set debug above, then we turn on the serial port
#ifdef DEBUG
   Serial.begin(115200);
   printf_begin();
#endif 
 
// Try to get the radio properties over the air
radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
// If that doesn't work, then set them from the settings provided in the configuration section
if(!OVER_THE_AIR_CONFIG_ENABLE)
   {
      int logicalControllerSequenceNum = 0;
      radio.AddLogicalController(logicalControllerSequenceNum, HARDCODED_START_CHANNEL, HARDCODED_NUM_CHANNELS,0);
   }

// Hang out a bit to let the radio get going	
delay(2);
  
// Let's check if the radio is up and receiving a carrier, if it is, we turn pin A0 on and light the status LED
if(radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID)){
  PORTC = PORTC | B00000001; // Set Pin 0 of port C (pin A0) HIGH to turn on the radio status LED
  }
  
// If we have DEBUG enabled, we print the radio details out to the serial port
#ifdef DEBUG
  radio.printDetails();   // Lets see what the radio settings are
#endif 

// Initialize data buffer
// This is how the rfShowControl data gets into the sketch
// buffer is an array that contains the DMX data
buffer= radio.GetControllerDataBase(0);	
delay (200);    // Hang out a little bit to let everything stabalize
}

/********************************************************************** End of Setup ********************************************************************/

// Function zeroCrossDetect
//
// The Zero Cross input will trigger this every time the sine wave crosses zero, no matter what the main loop is doing
// This will trigger at zero degrees and 180 degrees of the sine wave
void zeroCrossDetect() {
  if (dimLevel <= 400){          // why 400 ? it should never get this large
    noInterrupts();              // Disable interrupts to prevent the Zerocross from interrupting itself !
    dimLevel=(numSteps);         // Set Dim Level to the maximum numbers of steps (off)
    // Turn off the TRIAC Gates so if we cross zero they will turn off. Triacs stay latched on until they cross zero
    // We are writing directly to the output pins so make sure to define only the pins we use for Triacs
    // We use AND "&" here to allow any of non-triac pins to remain unchanged
    PORTB =  PORTB & B11111001;  // Sets Pins 1 and 2 of port B (5 and 6)  OFF, leaving other pins alone by way of the AND function 
    PORTD =  PORTD & B10011111;  // Sets Pins 5 and 6 of port D (9 and 10) OFF, leaving other pins alone by way of the AND function
    PORTC =  PORTC & B11100001;  // Sets Pins 1,2,3,4 of port C (A1 A2 A3 A4) OFF, leaving other pins alone by way of the AND function
    
    // Reset timer value attempt to stop it overfilling
    TCNT1 = 0;   //this may break timer1 in the future
    
    interrupts();  // We can turn interrupts back on now
    
    //flips status led on and off as needs based on interupt
    ledDataToggle();  // with real zero cross, this should be flashing every second
   
  }    
} 

// Function dimPointCheck
//
// When ever one of the dimming points are reached, we run this to see it it's time to fire the triacs
void dimPointCheck() {  
  if (dimLevel > 0){     // If dimLevel is zero, we don't have to do anything because the time step has counted down all the way
     dimLevel--;         // decrease time step counter by 1 step
  } 
 triggerOutputPin();    // Call the triggerOutputPin function to check the DMX data and fire the Triacs if needed
} 

// Function triggerOutputPin
//
// Here is where compare the DMX values to the current dimLevel and fire the triac if it's the correct time
void triggerOutputPin(){
//  We have the offset to give the triac time to unlatch and turn off
//  So what we have is that if dimLevel is 0-30, we are full ON , if it's 285-316 we ar full OFF
int dimOffsetLevel = dimLevel - offsetSteps;  // so in theory, the maximun here is 255
// So if we have a value between 1 and 255, we see if the DMX value is equal to or higher than our dimming level
// If it is , we turn the Triac on , writing directly to the Pins, be carefull if you make changes here
  if (dimOffsetLevel <= 255){
    if(dimOffsetLevel >= 1){ 
       //Channel 1
      if (buffer[0] >= dimOffsetLevel ){
        PORTD = PORTD | B00100000; // CH1 Sets Pin 5 of port D (pin 9) ON, leaving other pins alone by way of the OR function 
      }//Channel 2
      if (buffer[1] >= dimOffsetLevel){
        PORTD = PORTD | B01000000; // CH2 Sets Pin 6 of port D (pin 10) ON, leaving other pins alone by way of the OR function
      }//Channel 3
      if (buffer[2] >= dimOffsetLevel){
        PORTB = PORTB | B00000010; // CH3 Sets Pin 1 of port B (pin 5) ON, leaving other pins alone by way of the OR function
      }//Channel 4
      if (buffer[3] >= dimOffsetLevel){
        PORTB = PORTB | B00000100; // CH4 Sets Pin 2 of port B (pin 6) ON, leaving other pins alone by way of the OR function
      }//Channel 5
      if (buffer[4] >= dimOffsetLevel ){
        PORTC = PORTC | B00000010; // CH1 Sets Pin 1 of port C (pin A1) ON, leaving other pins alone by way of the OR function 
      }//Channel 6
      if (buffer[5] >= dimOffsetLevel){
        PORTC = PORTC | B00000100; // CH2 Sets Pin 2 of port C (pin A2) ON, leaving other pins alone by way of the OR function
      }//Channel 7
      if (buffer[6] >= dimOffsetLevel){
        PORTC = PORTC | B00001000; // CH3 Sets Pin 3 of port C (pin A3) ON, leaving other pins alone by way of the OR function
      }//Channel 8
      if (buffer[7] >= dimOffsetLevel){
        PORTC = PORTC | B00010000; // CH4 Sets Pin 4 of port C (pin A4) ON, leaving other pins alone by way of the OR function
      }
    }   
  }
}

// Function ledDataToggle
// Function to make the Status LED blink when zero crossing is detected and we have data to send out
void ledDataToggle(){
  if  (dataAvaliable > 0){
    if (dataLedCycle >= 60){
      PORTC =  PORTC | B00000001; // Turn A0 ON
    }else{
      PORTC =  PORTC & B11111110; // Turn A0 OFF
    }
    dataLedCycle = dataLedCycle - 1;
    dataAvaliable = dataAvaliable - 1;
    if (dataLedCycle == 0){
      dataLedCycle = 120;
    }
  }
}

//Main loop that runs when nothing is going on with interupts
void loop() {
  if(radio.Listen()){
    bufferOutput = buffer; // This sets up the channel shifting if start channel is not 1
    dataAvaliable = 120; //reset couter when we get data.
    // Set up to print out channel data if we are in DEBUG mode
    // This may mess up the trigger timing because of the delay induced, so only use 
    // it when debugging a channel data issue
    #ifdef DEBUG
    Serial.print("The channel Data for Channels 1 to 8 is   ");
    int i;
      for (i = 0; i < 8; i = i + 1) {
      Serial.print("\t");
      Serial.print(buffer[i]);
      }
    Serial.println("\t");
    #endif
   }
}
