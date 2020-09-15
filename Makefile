.PHONY: .arduinoavr, clean

.arduinoavr: 
	arduino-cli core install arduino:avr
	arduino-cli lib install Arduboy2

install: build/arduboy-chip8.arduino.avr.leonardo.hex
	./install-first.sh

roms/*:
	cp -r sampleroms roms

programs.h: roms/* dump.go
	go run dump.go roms > programs.h

arduboy: build/arduboy-chip8.arduino.avr.leonardo.hex

build/arduboy-chip8.arduino.avr.leonardo.hex: programs.h *.ino src/chip8/*.cpp src/chip8/*.hpp src/arduboy/*.cpp src/arduboy/*.hpp .arduinoavr
	arduino-cli compile -b arduino:avr:leonardo

sdl: build/sdl

build/sdl: programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ src/chip8/*.cpp sdl/*.cpp -I. -lSDL2 -std=c++11 -g -o build/sdl

sdl-compile: programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ -c src/chip8/*.cpp sdl/*.cpp -I. -std=c++11 


run-sdl: build/sdl
	./build/sdl

build:
	mkdir build

clean:
	rm -rf build


