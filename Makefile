.PHONY: .arduinoavr, clean

.arduinoavr: 
	arduino-cli core install arduino:avr

install: arduboy-chip8.arduino.avr.leonardo.hex
	./install-first.sh

roms/*:
	cp -r sampleroms roms

programs.h: roms/* dump.go
	go run dump.go roms > programs.h

arduboy-chip8.arduino.avr.leonardo.hex: programs.h *.ino src/chip8/*.cpp src/chip8/*.hpp *.h src/arduboy/*.cpp src/arduboy/*.hpp .arduinoavr
	arduino-cli compile -b arduino:avr:leonardo

build/sdl: programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ src/chip8/*.cpp sdl/*.cpp -I. -lSDL2 -std=c++11 -g -o build/sdl

run-sdl: build/sdl
	./build/sdl

build: arduboy-chip8.arduino.avr.leonardo.hex

clean:
	rm -rf build


