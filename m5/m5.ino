#include <M5Stack.h>
#include "render.hpp"
#include "src/chip8/chip8.hpp"
#include "src/chip8/simplemem.hpp"
#include "programs.h"
#include "src/arduino/tracer.hpp"
#include "gamepad.hpp"

M5Gamepad gamepad;

M5Render render(gamepad);
SimpleMemory memory;
SerialTracer tracer;
Chip8 emu(render, memory, tracer);

uint32_t next_step = 0;
uint32_t next_tick = 0;

uint8_t pidx = 0;

bool loaded = false;

// the setup routine runs once when M5Stack starts up
void setup() {
    M5.begin();
    M5.Lcd.wakeup();
    M5.Lcd.setBrightness(10); 
    M5.Lcd.setTextSize(2);

    Serial.begin(115200);
    Wire.begin();


    ledcWriteTone(TONE_PIN_CHANNEL, 0);
    ledcDetachPin(SPEAKER_PIN);
    dacWrite(25, 0);
    M5.Speaker.mute();
    printMenu();
}

void printMenu() {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0,0);
    String text = "    8Boy for M5Stack";
    if (pidx < 0) pidx = 0;
    if (pidx >= PROGRAM_COUNT) pidx = PROGRAM_COUNT;
    M5.Lcd.println(text);
    for(int i = 0; i < PROGRAM_COUNT; i++) {
        M5.Lcd.print(i == pidx ? ">" : " ");
        M5.Lcd.println(programs[i].name);
    }
}

void loadCurrentItem() {
    const Program &pgm = programs[pidx];
    render.setKeyMap(pgm.keymap[0], pgm.keymap[1], pgm.keymap[2]);
    memory.load(pgm.code, pgm.size);
    emu.SetConfig({
        .ShiftQuirk = pgm.shiftquirk 
    });
    emu.Reset();
    M5.Lcd.fillScreen(BLACK); 
    render.clear();
    loaded = true;
}

void pollLoader() {
    gamepad.poll();
    if(gamepad.justPressed(GAMEPAD_UP)) {
        pidx--;
        printMenu();
    }
    if(gamepad.justPressed(GAMEPAD_DOWN)) {
        pidx++;
        printMenu();
    } 
    if(gamepad.justPressed(GAMEPAD_A | GAMEPAD_B)) {
        loadCurrentItem();
    }
}

void handleError(ErrorType error) {
        M5.Lcd.setCursor(0,0);
        switch(error) {
            case STOPPED: 
                M5.Lcd.println(F("Program Exited"));
                M5.Lcd.println(F("Left: To Loader"));
                M5.Lcd.println(F("Other: Restart"));
                return;
            case STACK_UNDERFLOW: M5.Lcd.println(F("UNDERFLOW")); break;
            case STACK_OVERFLOW: M5.Lcd.println(F("OVERFLOW")); break;
            case OUT_OF_MEMORY: M5.Lcd.println(F("OUT OF MEM")); break;
            case BAD_READ: M5.Lcd.println(F("BAD READ")); break;
            case BAD_FETCH: M5.Lcd.println(F("BAD FETCH")); break;
            case UNIMPLEMENTED_INSTRUCTION: M5.Lcd.println(F("UNIMPL")); break;
        }
}

void runEmu() {
    if(gamepad.justPressed(GAMEPAD_SEL)) {
        emu.Reset();
        loaded = false;
        printMenu();
        return;
    }

    uint32_t now = micros();
    ErrorType error = emu.Step();
    if(error != NO_ERROR) {
        handleError(error);
    }
        
    if (now >= next_tick) {
        gamepad.poll();
        next_tick = now + 16666;
        emu.Tick();
    }
}

void loop() {
    if(loaded) {
        runEmu();
    } else {
        pollLoader();
    }
}
