#pragma once

#include <stdint.h>

class Memory {
    public: 
        // implementations should store the program data of the specified size.
        virtual void load(const uint8_t *program, const uint16_t size) = 0;
        // implementatiosn should return a byte in &val for the given address.
        // if it's not possible, return false.
        virtual bool read(uint16_t addr, uint8_t &val) = 0;
        // implementations should write the given value at the specified address.
        // if it's not possible, return false.
        virtual bool write(uint16_t addr, uint8_t val) = 0;
        // implementations should reset any state they need to keep (but not
        // necessarily clear all the memory)
        virtual void reset() = 0;
};
