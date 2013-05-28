RFPixelControl
==============

RFPixelControl - extension of the RF24 library for use with pixels

The RFPixel control serves as a common code configuration for both Receiver and Transmitter.

Hardware Support: 
Arduino Uno, Atmega 168, 328 
(Due coming soon,  porting for the kombyone transmitter board)

It works really well (I am very biased) on the RF1 Pixel series controllers 
http://www.komby.com/RF1
http://www.komby.com/RF1_12V


Pixels Supported:

For Receiver Devices - 
Currently Supported Pixel types / Libraries Used:

WM2999 - Holiday Time Color Changing Lights
Example Sketch: https://github.com/komby/RFPixelControl/tree/master/Examples/WM2999Receiver
Library Used: http://github.com/komby/wm2999

WS2801 - Adafruit Non-hardware SPI driver
Example Sketch: https://github.com/komby/RFPixelControl/tree/master/Examples/WS2801Receiver
Library Used: https://github.com/adafruit/Adafruit-WS2801-Library


Multiple :  (beta support)
LPD8806
SM16716
WS2801
TM1809
UCS1903
US2811
TM1803
Example Sketch: https://github.com/komby/RFPixelControl/tree/master/Examples/FastSPI_LED2Receiver
Library Used:  https://code.google.com/p/fastspi/downloads/list  (currently using FastSPI_LED2.Preview.20130325)


For Transmitter Devices: 

USB(FTDI) Open DMX transmitter (Joe Johnson)
This transmitter uses OpenDMX plugin in Vixen - It is called WM2999 but will work with all receiver types
https://github.com/komby/RFPixelControl/blob/master/Examples/WM2999Transmitter_3_6_2013_Beta/WM2999Transmitter_3_6_2013_Beta.ino

	To use the Transmitter you will need to make some changes to the HardwareSerial.cpp library in the arduino installation folder.
	That process has been discussed here: http://www.freaklabs.org/index.php/Tutorials/Software/Light-Sequencing-and-Decoding-DMX-with-an-Arduino.html
	as well as http://doityourselfchristmas.com/forums/showthread.php?22062-New-High-Speed-Wireless-GE-Color-Effects-Controller-RFColor2_4



DEBUG / Test Transmitter 
This transmitter utilizes the debug/rainbow methods provided in the Adafruit driver.  It will continuously stream RF data to the receivers.
https://github.com/komby/RFPixelControl/tree/master/Examples/TestRFTransmitter


E1.31 Transmitter Options

Arduino Ethernet (Wiznet) - Arduino Transmitter
Not a part of the RFPixelControl Library examples
http://doityourselfchristmas.com/forums/showthread.php?22062-New-High-Speed-Wireless-GE-Color-Effects-Controller-RFColor2_4/page57  (Stephen Morgan)


ENC28J60 - Arduino transmitter example.  
Its Crude, It works but only for a little while and then dies.  Needs some debugging to get working for production.
(ALPHA) https://github.com/komby/RFPixelControl/tree/master/Examples/E1_31Transmitter





Support Library ( without which none of this code would exist !!!!!!!!!!!!!!!!!!!!!!)
To use this library you will need to also get the RF24 Library
https://github.com/maniacbug/RF24
You do not need to make any changes to the RF24 library.

The RF24 wiring used was based on http://maniacbug.wordpress.com/2011/11/02/getting-started-rf24/


