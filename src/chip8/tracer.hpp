#pragma once

#include "../chip8/state.hpp"
#include "../chip8/errors.hpp"

class Tracer {
    public:
    virtual void exec(const EmuState &state) = 0; 
    virtual void error(ErrorType errorType, const EmuState &state) = 0; 
};

class NopTracer : public Tracer {
    virtual void exec(const EmuState &state) {}
    virtual void error(ErrorType errorType, const EmuState &state) {}
};

