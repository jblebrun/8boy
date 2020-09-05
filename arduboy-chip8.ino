#include <Arduboy2.h>

#include "src/chip8/chip8.hpp"

#include "programs.h"
#include "src/arduboy/render.hpp"
#include "src/arduboy/errors.hpp"
#include "src/arduboy/mem.hpp"


Arduboy2 boy;
ArduMem memory;

ArduboyErrors errors(boy);
ArduboyRender render(boy);
Chip8 emu(render, errors, memory);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    boy.setFrameRate(60);
}

uint8_t pidx;
const Program *program;
bool super = false;

unsigned long next = 0;
void runEmu() {
    if(boy.nextFrame()) {
        render.tick();
        emu.Tick();
    }
    
    unsigned long t = micros();
    if(t < next) {
        return;
    }
    next = t + (super ? 100 : 1000);
    if(boy.pressed(UP_BUTTON) &&
            boy.pressed(DOWN_BUTTON) && 
            boy.pressed(LEFT_BUTTON) && 
            boy.pressed(RIGHT_BUTTON)) {
        emu.Reset();
        program = NULL;
    }

    bool running = emu.Step();

    // If the emulator exited, we show a message that pressing a key will
    // restart it. So, handle that here.
    if(!running & boy.justPressed(0xFF)) emu.Reset();
}

void runLoader() {
    boy.clear();
    boy.setCursor(0,0);
    super = pgm_read_byte(&programs[pidx].super);
    boy.print(F("Select: "));
    boy.setCursor(0,30);
    boy.print(pidx);
    boy.setCursor(16,30);
    char buffer[16];
    strcpy_P(buffer, (const char*)pgm_read_ptr(&programs[pidx].name));
    boy.println(buffer);
    if(super) {
        boy.println(F("SCHIP-8"));
    } else {
        boy.println(F("CHIP-8"));
    }
    strcpy_P(buffer, (const char*)pgm_read_ptr(&programs[pidx].info));
    boy.println(buffer);
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
        render.setKeyMap(
            pgm_read_byte(&(program->keymap[0])),
            pgm_read_byte(&(program->keymap[1])),
            pgm_read_byte(&(program->keymap[2]))
        );
        memory.load(code, size);

        // Wait for button release before starting emulator, to avoid 
        // the loader button press from registering in the game.
        while(boy.pressed(A_BUTTON));

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
