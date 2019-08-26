#include "chip8.hpp"

const uint8_t font[] PROGMEM = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

const uint8_t fonthi[] PROGMEM = {
    0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

// Try to find an already-allocated slab for the provided address.
// This works by searching through the current list of slabs for as
// long as we see slabs with a page identifier != 0.
// If a slab is found, a pointer to it is returned.
// If we reach an unused slab, we return that, the caller can decide
// to allocate it if desired.
// If the end of the list is reached with no match, NULL is returned.
Slab* Chip8::findSlab(uint16_t addr) {
    uint8_t page = addr >> 4;
    for(uint8_t i = 0; i < SLAB_COUNT; i++) {
        if(mSlabs[i].page == page || mSlabs[i].page == 0) {
            return &mSlabs[i];
        }
    }
    return NULL;
}

// readMem tries to read memory from the right place
// for the specified address.
// (1) look for an allocated slab, and uses that.
// (2) if the address in the low space, assume it's for a font, use that.
// (3) If the adddress is in the program space, read from the program in flash.
// (4) Uh - oh, something wasn't considered. Halt and show a message.
uint8_t Chip8::readMem(uint16_t addr) {
    Slab* slab = findSlab(addr);
    if(slab && slab->page != 0) {
        return slab->data[addr&0xF];
    }

    if(addr < (sizeof(font) + sizeof(fonthi))) {
        return pgm_read_byte(&fonthi[addr]);
    }

    if(addr < sizeof(font)) {
        return pgm_read_byte(&font[addr]);
    }

    if(addr < mProgramSize) {
        return pgm_read_byte(&mProgram[addr-0x200]);
    } 

    mRender.badread(mPC-2);
    mRunning = false;
    return 0;
}

// writeMem finds or allocates a slab of memory and
// updates the provided value in it.
// If the slab allocator is full and we need a new one,
// the program halts and the screen displays a message,
// the provided program will need more slabs to run successfully.
// If an unallocated page is returned, its page is set to the
// page for the requested address. 
// If the page overlaps with any program memory, the program
// memory is copied into the slab.
void Chip8::writeMem(uint16_t addr, uint8_t val) {
    Slab *slab = findSlab(addr);
    if(!slab) {
        mRender.oom(mPC-2);
        mRunning = false;
        return;
    }

    // If slab is new, set it up
    if(slab->page == 0) {
        slab->page = addr >> 4;
        uint16_t pageStart = addr & 0xFF0;
        for(uint8_t i = 0; i < 16; i++) {
            // Initialize the slab to either 0, or the corresopnding program memory
            if(pageStart + i < mProgramSize) {
                slab->data[i] = pgm_read_byte(&mProgram[(pageStart|i)-0x200]);
            } else {
                slab->data[i] = 0;
            }
        }
    }

    slab->data[addr&0xF] = val;
}
