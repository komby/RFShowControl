RFPixelControl
==============

RFPixelControl - extension of the RF24 library for use with pixels


The RFPixel control serves as a common code configuration for both RX and TX functions.
It utilizes the RF24 library.

This Code was created to support the WM2999 project but built so that it could be used as a pixel implementation dependent code.
You can read more about that project here
http://doityourselfchristmas.com/forums/showthread.php?24162-Hackable-Cheap-Walmart-quot-Pixels-quot-an-exlectronic-puzzle

To use this library you will need to also get the RF24 Library

https://github.com/maniacbug/RF24

You do not need to make any changes to the RF24 library.

To use the Transmitter you will need to make some changes to the HardwareSerial.cpp library in the arduino installation folder.

That process has been discussed here: http://www.freaklabs.org/index.php/Tutorials/Software/Light-Sequencing-and-Decoding-DMX-with-an-Arduino.html

as well as http://doityourselfchristmas.com/forums/showthread.php?22062-New-High-Speed-Wireless-GE-Color-Effects-Controller-RFColor2_4


The RF24 wiring used was based on http://maniacbug.wordpress.com/2011/11/02/getting-started-rf24/


