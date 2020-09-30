#pragma once

#include "../chip8/state.hpp"
#include "../chip8/errors.hpp"
#include "../chip8/config.hpp"

// Hooks for tracing execution of the emulator. Default implemenation does nothing.
class Tracer {
    public:
    // Called just before execution of an instruction begins.
    virtual inline void exec(const EmuState &state, const Config &config) {}
    // Called just after execution of an instruction completes.
    virtual inline void execFinished(const EmuState &state, const Config &config) {} 
    // Called when execution of an instruction results in an error.
    virtual inline void error(ErrorType errorType, const EmuState &state, const Config &config) {}
};
