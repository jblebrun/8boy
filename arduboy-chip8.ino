#include <Arduboy2.h>
#include "chip8.hpp"

uint16_t maze[] = {
    0x6000,
    0x6100,
    0xa222, //0x0004
    0xc201,
    0x3201,
    0xa21e,
    0xd014,
    0x7004,

    0x3040, //0x10
    0x1204,
    0x6000,
    0x7104,
    0x3120,
    0x1204,
    0x121c,
    //maze data 
    0x8040, //0x1e

    0x2010,
    0x2040, //0x22
    0x8010,
};

Arduboy2 boy;
Chip8 emu(boy);

void setup() {
    boy.begin();
    boy.initRandomSeed();
    Serial.begin(115200);
    emu.Load(maze);
    while(!Serial);
}

void loop() {
    emu.Step();

    if(boy.pressed(B_BUTTON)) {
        emu.Reset();
    }
}
