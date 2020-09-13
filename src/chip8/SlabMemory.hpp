#pragma once

#include "../chip8/memory.hpp"

#define SLAB_SIZE 16

struct Slab {
    uint8_t page;
    uint8_t data[SLAB_SIZE];
};

// Provides a slab-based memory allocation strategy.
//
// This is an abstract class, to use it, implement it and provide a way to read
// data from some external memory source. 
//
//  When data is written to memory by a program, one or more 16-byte
// slabs will be allocated to accept the memory writes. Subsequent reads from
// addresses covered by those slabs will always be servied by the slabs, so your
// implementation-provided data will be shadowed, not updated.
//
// All slabs are preallocated at startup, there are 40 slabs available, or
// about 640 bytes. If a program writes to enough areas that more than 40 slabs
// are needed, then an out of memory error will be returned.
//
// While it's possible to construct a program that would run on the original machine, 
// but results in an OOM here, this implementation is likely to provide enough
// RAM workspace for most real-world Chip8 games.
class SlabMemory : public Memory {
    // Slabs for dynamic memory allocation.
    Slab *mSlabs;

    uint16_t mSlabCount;


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

    // When subclassing, implement this to return non-RAM memory assets mapped
    // to a particular address, like fonts and program data.
    virtual bool externalRead(uint16_t addr, uint8_t* dest, uint8_t size) = 0;

    
    public:
        // Create a SlabMemory using the provided pointer to a contiguous memory
        // region that will be treated as an array of slabs of the provided size.
        SlabMemory(Slab* slabStart, uint16_t slabCount);
        void reset();
        bool read(uint16_t addr, uint8_t *dst, uint8_t size);
        bool write(uint16_t addr, uint8_t *src, uint8_t size);
};
