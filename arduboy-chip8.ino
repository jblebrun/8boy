#include <Arduboy2.h>

#include "src/chip8/chip8.hpp"

#include "programs.h"
#include "src/arduboy/render.hpp"
#include "src/arduboy/mem.hpp"


Arduboy2 boy;
ArduMem memory;

ArduboyRender render(boy);
Chip8 emu(render, memory);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    boy.setFrameRate(60);
}

uint8_t pidx;
const Program *program;
bool super = false;

void printWord(uint16_t w) {
    boy.print(F("0x"));
    if((w & 0xF000) == 0) boy.print(0);
    if((w & 0xFF00) == 0) boy.print(0);
    if((w & 0xFFF0) == 0) boy.print(0);
    boy.println(w, HEX);
}

unsigned long next = 0;
void runEmu() {
    if(boy.nextFrame()) {
        render.tick();
        emu.Tick();
    }

    // If the emulator exited, we show a message that pressing a key will
    // restart it. So, handle that here.
    if(!emu.Running()) {
        boy.pollButtons();

        // Any key will reset
        if(boy.justPressed(0xFF)) {
            emu.Reset();
        }

        // Left also drops back to loader
        if(boy.justPressed(LEFT_BUTTON)) {
            program = NULL;
        }

        return;
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

    ErrorType error = emu.Step();
    
    if(error != NO_ERROR) {
        boy.setCursor(0,0);
        switch(error) {
            case STOPPED: 
                boy.println(F("Program Exited"));
                boy.println(F("Left: To Loader"));
                boy.println(F("Other: Restart"));
                return;
            case STACK_UNDERFLOW: boy.println(F("UNDERFLOW")); break;
            case STACK_OVERFLOW: boy.println(F("OVERFLOW")); break;
            case OUT_OF_MEMORY: boy.println(F("OUT OF MEM")); break;
            case BAD_READ: boy.println(F("BAD READ")); break;
            case BAD_FETCH: boy.println(F("BAD FETCH")); break;
            case UNIMPLEMENTED_INSTRUCTION: boy.println(F("UNIMPL")); break;
        }
        uint16_t pc = emu.GetPC();
        boy.print(F("PC: "));
        printWord(pc);
        boy.print(F("INST: "));
        uint16_t inst;
        emu.ReadWord(pc, inst);
        printWord(inst);
    }

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
