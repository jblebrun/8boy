#include "../chip8/tracer.hpp"
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
        virtual void exec(const EmuState &state);
        virtual void error(ErrorType errorType, const EmuState &state); 
};

