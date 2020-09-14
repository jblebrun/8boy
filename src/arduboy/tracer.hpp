#include "../chip8/tracer.hpp"
#include "../chip8/chip8.hpp"
#include "PrintHelper.hpp"
#include <Arduino.h>

class SerialTracer : public Tracer {
    PrintHelper mPrint;

    __FlashStringHelper* errorMessage(ErrorType errorType);

    bool mTracing = false;

    public:
        SerialTracer(bool enabled = false) : 
            mPrint(PrintHelper(Serial)),
            mTracing(enabled) {}
        virtual void exec(const EmuState &state, const Config &config);
        virtual void error(ErrorType errorType, const EmuState &state, const Config &config); 
};

