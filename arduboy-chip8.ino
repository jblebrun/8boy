#include <Arduboy2.h>
#include "chip8.hpp"
#include "programs.h"

Arduboy2 boy;
Chip8 emu(boy);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    boy.setFrameRate( 60);
    emu.Load((const uint8_t*)pgm_read_ptr(&programs[1]));
    emu.Reset();
    //while(!Serial);
}

void loop() {

    if(!boy.pressed(UP_BUTTON) &&
            boy.pressed(DOWN_BUTTON) && 
            boy.pressed(LEFT_BUTTON) && 
            boy.pressed(RIGHT_BUTTON)) {
        emu.Reset();
    }

    boy.pollButtons();
    emu.Buttons(boy.buttonsState());
    if(boy.justPressed(A_BUTTON)) {
        emu.Toggle();
    }
    if(emu.Running()) {
        emu.Step();
    }
}
