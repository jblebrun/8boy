#pragma once

#include "../chip8/memory.hpp"
#include <Arduino.h>

#define SLAB_SIZE 16

struct Slab {
    uint8_t page;
    uint8_t data[SLAB_SIZE];
};

#define SLAB_COUNT 40

// Provides a memory for the Arduboy. Program and fonts are provided via PROGMEM 
// arrays. When data is written to memory by a program, one or more 16-byte
// slabs will be allocated to accept the memory writes. Subsequent reads from
// addresses covered by those slabs will always be servied by the slabs, so
// font and program data can be modified.
//
// All slabs are preallocated at startup, there are 40 slabs available, or
// about 640 bytes. If a program writes to enough areas that more than 40 slabs
// are needed, then an out of memory error will be returned.
//
// While it's possible to construct a program that would run on the original machine, 
// but results in an OOM here, this implementation is likely to provide enough
// RAM workspace for most real-world Chip8 games.
class ArduMem : public Memory {
    // The bytes for the program, which reside in PROGMEM
    const uint8_t *mProgram;

    // The size of mProgram
    uint16_t mProgramSize;

    // Slabs for dynamic memory allocation.
    Slab mSlabs[SLAB_COUNT];

    // Initialize the provided slab, filling it with either 0, or the value of
    // the address from PROGMEM, if there is a mapping.
    void initSlab(Slab &slab, uint16_t addr);

    // Find a write slab that can service the provided address. If there isn't one
    // for the page of the requested address, and some are still available, a new 
    // one will be initialized and returned.
    Slab* findWriteSlab(uint16_t);

    // Find the first slab in the provided base + size range, if there is one, else
    // return null.
    Slab* firstReadSlab(uint16_t, uint8_t);

    // Attempt to read data from PROGMEM resources for the requested range. If the
    // entire range can't be filled with data from PROGMEM items, it returns null.
    bool pgmRead(uint16_t, uint8_t*, uint8_t);
    
    public:
        ArduMem() {};
        virtual void load(const uint8_t *program, const uint16_t size);
        virtual bool read(uint16_t addr, uint8_t *dst, uint8_t size);
        virtual bool write(uint16_t addr, uint8_t *src, uint8_t size);
        virtual void reset();
};
