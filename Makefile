.PHONY: .arduinoavr, .arduinoesp, clean

ESP32_URL= https://dl.espressif.com/dl/package_esp32_index.json

default: install-arduboy

# Common
roms/*:
	cp -r sampleroms roms

programs.h: roms/* tools/dump.py tools/op.py
	python3 tools/dump.py roms > programs.h

clean:
	rm -rf build

# Arduboy
.arduinoavr: 
	arduino-cli core install arduino:avr
	arduino-cli lib install Arduboy2

arduboy: build/arduboy-chip8.arduino.avr.leonardo.hex

build/arduboy-chip8.arduino.avr.leonardo.hex: programs.h src/chip8/*.cpp src/chip8/*.hpp arduboy/*.ino arduboy/*.cpp arduboy/*.hpp .arduinoavr
	cd arduboy; arduino-cli compile -b arduino:avr:leonardo

install-arduboy: build/arduboy-chip8.arduino.avr.leonardo.hex
	cd arduboy; ../tools/install-first.sh Leonardo arduino:avr:leonardo


#M5
.arduinoesp:
	arduino-cli core --additional-urls=$(ESP32_URL) install esp32:esp32
	arduino-cli lib install M5Stack

install-m5: m5
	cd m5; ../tools/install-first.sh SLAB_USB esp32:esp32:esp32

m5: m5/build/m5.arduino.esp32.esp32.esp32.hex

m5/build/m5.arduino.esp32.esp32.esp32.hex: programs.h src/chip8/*.cpp src/chip8/*.hpp m5/*.ino .arduinoesp
	cd m5; arduino-cli compile -b esp32:esp32:esp32


# SDL Version
sdl: build/sdl

build/sdl: program.h programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ src/chip8/*.cpp sdl/*.cpp -I. -lSDL2 -std=c++11 -g -o build/sdl

sdl-compile: program.h programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ -c src/chip8/*.cpp sdl/*.cpp -I. -std=c++11 


run-sdl: build/sdl
	./build/sdl



