#include <Arduboy2.h>
#include "chip8.hpp"
#include "programs.h"

Arduboy2 boy;
Chip8 emu(boy);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    emu.Load((const uint8_t*)pgm_read_ptr(&programs[0]));
    while(!Serial);
}

void loop() {

    boy.pollButtons();
    if(boy.justPressed(B_BUTTON)) {
        emu.Reset();
    }

    if(boy.justPressed(A_BUTTON)) {
        emu.Toggle();
    }
    if(emu.Running()) {
        emu.Step();
    }
}
