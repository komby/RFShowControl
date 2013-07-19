BOARD_TAG            ?= uno
ARDUINO_PORT         ?= $(firstword $(wildcard /dev/ttyACM* /dev/ttyUSB*))
ARDUINO_LIBS         ?= Wire Wire/utility RF24 RFPixelControl SPI
ARDUINO_DIR          ?= $(HOME)/toolchains/arduino
ARDMK_DIR            ?= ../Arduino-Makefile

AVRDUDE              ?= avrdude
AVR_TOOLS_DIR        ?= $(ARDUINO_DIR)/hardware/tools/avr
AVRDUDE_CONF         ?= /etc/avrdude.conf
AVRDUDE_ARD_BAUDRATE ?= 115200

include $(ARDMK_DIR)/arduino-mk/Arduino.mk
