#pragma once

#include "../chip8/state.hpp"
#include "../chip8/errors.hpp"
#include "../chip8/config.hpp"

class Tracer {
    public:
    virtual void exec(const EmuState &state, const Config &config) = 0; 
    virtual void error(ErrorType errorType, const EmuState &state, const Config &config) = 0; 
};

class NopTracer : public Tracer {
    virtual void exec(const EmuState &state, const Config &config) {}
    virtual void error(ErrorType errorType, const EmuState &state, const Config &config) {}
};

