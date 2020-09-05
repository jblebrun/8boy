#pragma once

#include <stdint.h>

// The interface that a platform will need to implement in order to handle the various
// errors that may arise while executing a chip8 program.
class Errors {
    public:
    // This will be called when a return is executed with no more stack entries.
    virtual void stackUnderflow(uint16_t addr) {};

    // This will be called if calls are more than 16 levels deep.
    virtual void stackOverflow(uint16_t addr) {};

    // This will be called if the underlying platform couldn't allocated the memory
    // needed for a write.
    virtual void oom(uint16_t addr) {};

    // This will be called if a memory read failed to return a value for the
    // provided address.
    virtual void badread(uint16_t addr) {};

    // This will be called if there's no implementation for a chip8 instruction.
    virtual void unimpl(uint16_t addr, uint16_t inst) {};
};
