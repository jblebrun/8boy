#include <M5Stack.h>
#include <Wire.h>

#include "gamepad.hpp"

#define FACES_GAMEPAD_I2C_ADDR 0x08

void M5Gamepad::poll() {
    Wire.requestFrom(FACES_GAMEPAD_I2C_ADDR, 1);
    if(Wire.available()) {
        uint8_t next = ~(Wire.read()); 
        mJustPressed = ~mPressed & next;
        mJustReleased = mPressed & ~next;
        mPressed = next;
    }
}

bool M5Gamepad::pressed(uint8_t which) {
    return (which & mPressed) > 0; 
}

bool M5Gamepad::justPressed(uint8_t which) {
    return (which & mJustPressed) > 0;
}
