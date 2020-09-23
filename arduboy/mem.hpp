#pragma once

#include "src/chip8/SlabMemory.hpp"
#include <Arduino.h>

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
class ArduMem : public SlabMemory {

    // The bytes for the program, which reside in PROGMEM
    const uint8_t *mProgram;

    // The size of mProgram
    uint16_t mProgramSize;

    // The slabs provided to the base class.
    Slab mSlabs[SLAB_COUNT];

    bool externalRead(uint16_t addr, uint8_t* dest, uint8_t size);

    public:
        ArduMem();
        void load(const uint8_t *program, uint16_t size);
};
