BOARD_TAG    = uno
ARDUINO_DIR  = /usr/share/arduino
ARDMK_DIR    = /usr/share/arduino
MONITOR_PORT = /dev/arduino

CXXFLAGS += -I./MIDI

ARDUINO_QUIET = true
include ${ARDMK_DIR}/Arduino.mk
