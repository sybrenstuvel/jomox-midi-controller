CC=avr-gcc
CXX=avr-g++
LDD=avr-ld

ARDUINO=/usr/share/arduino/hardware/arduino
CFLAGS=-I./MIDI -I/usr/lib/avr/include -I${ARDUINO}/cores/arduino -I${ARDUINO}/variants/standard

CFLAGS+=-mmcu=avr5 -D__AVR_ATmega328P__
LDFLAGS=-static -mmcu=avr5

CXXFLAGS=${CFLAGS}

midi_jomox_interface: midi_jomox_interface.o

clean:
	rm -f *.o

.PHONY: clean

