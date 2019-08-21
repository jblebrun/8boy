#include <Arduboy2.h>
#include "chip8.hpp"
#include "programs.h"

Arduboy2 boy;
Chip8 emu(boy);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    boy.setFrameRate(60);
    //while(!Serial);
}

uint8_t pidx;
const Program *program;

void runEmu() {
    if(boy.pressed(UP_BUTTON) &&
            boy.pressed(DOWN_BUTTON) && 
            boy.pressed(LEFT_BUTTON) && 
            boy.pressed(RIGHT_BUTTON)) {
        emu.Reset();
        program = NULL;
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

const char name[] PROGMEM = "NAME";
void loop() {

    if (program == NULL) {
        boy.clear();
        boy.setCursor(0,0);
        boy.print(F("Select:"));
        boy.setCursor(0,30);
        boy.print(pidx);
        boy.print(" ");
        char buffer[10];
        strcpy_P(buffer, (const char*)pgm_read_ptr(&programs[pidx].name));
        boy.print(buffer);
        boy.display();
    
        boy.pollButtons();
        if(boy.justPressed(DOWN_BUTTON)) {
            pidx++;
            if(pidx >= PROGRAM_COUNT) {
                pidx = 0;
            }
        }
        if(boy.justPressed(UP_BUTTON)) {
            pidx--;
            if(pidx >= PROGRAM_COUNT) {
                pidx = PROGRAM_COUNT-1;
            }
        }
        if(boy.justPressed(A_BUTTON)) {
            program = &programs[pidx];
            uint16_t size = pgm_read_word(&(program->size));
            const uint8_t* code = pgm_read_ptr(&(program->code));
            emu.Load(code, size);
            emu.Reset();
            
        }
    } else {
        runEmu();
    }
}
