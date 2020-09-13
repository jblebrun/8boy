#include "simplemem.hpp"
#include "stdio.h"
#include "string.h" 
#include "font.hpp"

SimpleMemory::SimpleMemory() {
    memcpy(mMemory, font, sizeof(font));
    memcpy(mMemory + sizeof(font), fonthi, sizeof(fonthi));
}

void SimpleMemory::load(const uint8_t *program, const uint16_t size) {
    const uint8_t* end = program+size;
    if(size+0x200 > SIZE) {
        end = program+SIZE-0x200;
    }
    memcpy(mMemory+0x200, program, size);
}
        
bool SimpleMemory::read(uint16_t addr, uint8_t *dest, uint8_t size) {
    memcpy(dest, mMemory+addr, size);
    return true;
}
        
bool SimpleMemory::write(uint16_t addr, uint8_t *src, uint8_t size) {
    memcpy(mMemory+addr,src, size);
    return true;
}
