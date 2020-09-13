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
        // Populate the memory starting at *dest with the memory values at 
        // address addr. If The read can't be satisfied, returns false.
        virtual bool read(uint16_t addr, uint8_t *dest, uint8_t size) = 0;

        // Write size bytes starting at src to the memory starting at addr. If the
        // underlying implementation can't allocate enough memory to satisfy the 
        // write, false is returned. Some data may have been written.
        virtual bool write(uint16_t addr, uint8_t *src, uint8_t size) = 0;
};
