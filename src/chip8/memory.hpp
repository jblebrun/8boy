#pragma once

#include <stdint.h>

// Platforms should implement these methods to provide the needed memory
// read/write behavior for the platform. For platforms with more than 4k, this
// may be as simple as reading and writing from a single array.
//
// This abstraction was introduced to allow providing an alternate memory
// implementation for the Arduboy, which doesn't have enough RAM to use a flat
// direct addressing model.
class Memory {
    public: 
        // The data in `program` of size `size` should be loaded into the program
        // location in memory (typically, 0x200).
        virtual void load(const uint8_t *program, const uint16_t size) = 0;

        // implementatiosn should return a byte in &val for the given address.
        // if it's not possible, return false.
        virtual bool read(uint16_t addr, uint8_t &val) = 0;

        // implementations should write the given value at the specified address.
        // if it's not possible, return false.
        virtual bool write(uint16_t addr, uint8_t val) = 0;

        // implementations should reset any internal implementation-specific state that
        // they keep. It's not necessary to clear all of the actual memory.
        virtual void reset() = 0;
};
