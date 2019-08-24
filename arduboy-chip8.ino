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

unsigned long next = 0;
void runEmu() {
    unsigned long t = micros();
    if(t < next) {
        return;
    }
    next = t + 100;
    if(boy.pressed(UP_BUTTON) &&
            boy.pressed(DOWN_BUTTON) && 
            boy.pressed(LEFT_BUTTON) && 
            boy.pressed(RIGHT_BUTTON)) {
        emu.Reset();
        program = NULL;
    }

    boy.pollButtons();
    if(boy.justPressed(A_BUTTON)) {
        emu.Toggle();
    }
    emu.Buttons(boy.buttonsState());
    if(emu.Running()) {
        emu.Step();
    }
}

void runLoader() {
    boy.clear();
    boy.setCursor(0,0);
    bool super = pgm_read_byte(&programs[pidx].super);
    boy.print(F("Select:"));
    boy.setCursor(0,30);
    boy.print(pidx);
    boy.print(" ");
    char buffer[16];
    strcpy_P(buffer, (const char*)pgm_read_ptr(&programs[pidx].name));
    boy.print(buffer);

    boy.setCursor(0,50);
    if(super) {
        boy.print("SCHIP-8");
    } else {
        boy.print("CHIP-8");
    }
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
}

void loop() {
    if (program == NULL) {
        runLoader(); 
    } else {
        runEmu();
    }
}
