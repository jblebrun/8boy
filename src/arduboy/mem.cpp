#include "mem.hpp"
#define FONT_STORAGE_MODIFIER PROGMEM
#include "../chip8/font.hpp"

ArduMem::ArduMem() : SlabMemory(mSlabs, SLAB_COUNT) {} 

// The data in `program` of size `size` should be loaded into the program
// location in memory (typically, 0x200).
void ArduMem::load(const uint8_t *program, const uint16_t size)  {
    mProgram = program;
    mProgramSize = size;
}

bool ArduMem::externalRead(uint16_t addr, uint8_t *dest, uint8_t size) {
    if(size == 0) return true;

    uint8_t *src = NULL;

    if(addr + size < sizeof(font)) {
        // Fully in font space, read font. 
        src = &font[addr];
    } else if(addr < sizeof(font) + sizeof(fonthi)) {
        // Fully in fonthi space, read fonthi.
        src = &fonthi[addr - sizeof(font)];
    } else if(addr >= 0x200 & addr + size - 0x200 < mProgramSize) {
        // Fully in program space, read program.
        src = &mProgram[addr-0x0200];
    } 

    if(!src) return false;

    memcpy_P(dest, src, size);
    return true;
}
