#pragma once
#include <Arduboy2.h>
#include "src/chip8/errors.hpp"

class ArduboyErrors : public Errors {
    Arduboy2 &mBoy;

    public:
    ArduboyErrors(Arduboy2 &boy);
    // errors
    // override this implementations if you want to perform custom
    // behavior when the emulator encounters an error
    // emulator will be in stopped state 
    virtual void stackUnderflow(uint16_t addr);
    virtual void stackOverflow(uint16_t addr);
    virtual void oom(uint16_t addr);
    virtual void badread(uint16_t addr);
    virtual void unimpl(uint16_t addr, uint16_t inst);
};
