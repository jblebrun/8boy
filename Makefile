.PHONY: .arduinoavr, .arduinoesp, clean

ESP32_URL= https://dl.espressif.com/dl/package_esp32_index.json

default: install-arduboy

# Common
roms/*:
	cp -r sampleroms roms

programs.h: roms/* dump.go
	go run dump.go roms > programs.h

clean:
	rm -rf build

# Arduboy
.arduinoavr: 
	arduino-cli core install arduino:avr
	arduino-cli lib install Arduboy2


.arduinoesp:
	arduino-cli core --additional-urls=$(ESP32_URL) install esp32:esp32
	arduino-cli lib install M5Stack

arduboy: build/arduboy-chip8.arduino.avr.leonardo.hex

build/arduboy-chip8.arduino.avr.leonardo.hex: programs.h src/chip8/*.cpp src/chip8/*.hpp arduboy/*.ino arduboy/*.cpp arduboy/*.hpp .arduinoavr
	cd arduboy; arduino-cli compile -b arduino:avr:leonardo

install-arduboy: build/arduboy-chip8.arduino.avr.leonardo.hex
	cd arduboy; ../install-first.sh Leonardo


#M5
install-m5: m5
	cd m5; ../install-first.sh M5

m5: m5/build/m5.arduino.esp32.esp32.esp32.hex

m5/build/m5.arduino.esp32.esp32.esp32.hex: programs.h src/chip8/*.cpp src/chip8/*.hpp m5/*.ino .arduinoesp
	cd m5; arduino-cli compile -b esp32:esp32:esp32


# SDL Version
sdl: build/sdl

build/sdl: programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ src/chip8/*.cpp sdl/*.cpp -I. -lSDL2 -std=c++11 -g -o build/sdl

sdl-compile: programs.h src/chip8/*.cpp src/chip8/*.hpp sdl/*.cpp sdl/*.hpp 
	g++ -c src/chip8/*.cpp sdl/*.cpp -I. -std=c++11 


run-sdl: build/sdl
	./build/sdl



