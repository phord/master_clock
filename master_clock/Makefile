# Arduino build and program directives
# Uses https://github.com/sudar/Arduino-Makefile
# On Debian Linux, sudo apt-get install arduino-mk python-serial

BOARD_TAG     = mega2560
MONITOR_PORT  = $(lastword $(wildcard /dev/ttyACM*))
ARDUINO_LIBS = ArduinoEthernet SPI TimerOne Ethernet

# If youre ARDMK_DIR is somewhere you can override this variable on the make commandline 
#   make -f makefile-avr ARDMK_DIR=C:/some/other/path
# or in your environment
#   ARDMK_DIR=/home/me/arduino-mk/ && make -f makefile-avr
ARDMK_DIR ?= /usr/share/arduino/

avr: all

include $(ARDMK_DIR)/Arduino.mk
