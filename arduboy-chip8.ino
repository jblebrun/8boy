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

// Print out 5 program names, or all of them if less than 5. 
// If there are less than 5, the existing ones are weighted towards the top.
// Display a '>' cursor next to the one that's considered selected.
void printMenu() {
    boy.clear();
    boy.setCursor(0,0);

    char buffer[16];

    // our window shows up to 5 items:
    // current, 2 above, 2 below
    // If we're pointing at 0-2, our window starts from item 0. Otherwise it
    // starts from 2 before where we're pointing.
    // Moving down like this:
    // >Item 1   Item 1   Item 1   Item 2
    //  Item 2  >Item 2   Item 2   Item 3
    //  Item 3   Item 3  >Item 3  >Item 4
    //  Item 4   Item 4   Item 4   Item 5
    //  Item 5   Item 5   Item 5   Item 6
    uint8_t startIdx = pidx < 2 ? 0 : pidx - 2;

    // If we're near the end, we should just show the last 5 programs.
    // The cursor will finally progress to the last:
    //  Item N-4   Item N-4   Item N-4
    //  Item N-3   Item N-3   Item N-3
    // >Item N-2   Item N-2   Item N-2
    //  Item N-1  >Item N-1   Item N-1
    //  Item N     Item N    >Item N
    if(startIdx + 5 > PROGRAM_COUNT) startIdx = PROGRAM_COUNT - 5;

    // If it's a short list, just show everything.
    if(PROGRAM_COUNT <= 5) startIdx = 0;

    // Weighted towards the top, so a short list is like:
    // |------------
    // |Item 1
    // |item 2
    // |
    // | 
    // |
    // |
    // |TYPE
    // |INFO
    // |-------------
    for(uint8_t i = 0; i < 5; i++) {
        uint8_t lidx = i + startIdx;
        if(lidx < PROGRAM_COUNT) {
            buffer[0] = lidx == pidx ? '>' : ' ';  
            strncpy_P(&buffer[1], (const char*)pgm_read_ptr(&programs[lidx].name), 16);
            boy.println(buffer);
            
        } else {
            boy.println("");
        }
    }

    // Space blank line.
    boy.println("");

    // Type of program.
    super = pgm_read_byte(&programs[pidx].super);
    if(super) {
        boy.println(F("SCHIP-8"));
    } else {
        boy.println(F("CHIP-8"));
    }

    // Info from program, if any was included.
    strcpy_P(buffer, (const char*)pgm_read_ptr(&programs[pidx].info));
    boy.println(buffer);
    boy.display();
}

void loadCurrentItem() {
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

// This runs once per device loop, while in the game loading phase, to update
// the display and check inputs.
void runLoader() {
    // Show the menu, reflecting the current selection state.
    printMenu();

    // Handle moving up and own in the menu.
    boy.pollButtons();
    if(boy.justPressed(DOWN_BUTTON) && pidx < PROGRAM_COUNT - 1) pidx++; 
    if(boy.justPressed(UP_BUTTON) && pidx > 0) pidx--;

    // Handle selection.
    if(boy.justPressed(A_BUTTON|B_BUTTON)) loadCurrentItem();
}

void loop() {
    if (program == NULL) {
        runLoader(); 
    } else {
        runEmu();
    }
}
