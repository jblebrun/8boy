#pragma once

#include "src/chip8/memory.hpp"
#include <Arduino.h>

#define SLAB_SIZE 16

struct Slab {
    uint8_t page;
    uint8_t data[SLAB_SIZE];
};

#define SLAB_COUNT 40

class ArduMem : public Memory {
    const uint8_t *mProgram;
    uint16_t mProgramSize;
    Slab mSlabs[SLAB_COUNT];

    Slab* findSlab(uint16_t);

    public:
        ArduMem() {};
        virtual void load(const uint8_t *program, const uint16_t size);
        virtual bool read(uint16_t addr, uint8_t &val);
        virtual bool write(uint16_t addr, uint8_t val);
        virtual void reset();
};
