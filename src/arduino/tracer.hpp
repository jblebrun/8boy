#include "../chip8/tracer.hpp"
#include "../chip8/chip8.hpp"
#include "PrintHelper.hpp"
#include <Arduino.h>

class SerialTracer : public Tracer {
    PrintHelper mPrint;

    const __FlashStringHelper* errorMessage(ErrorType errorType);

    uint32_t mExecStart = 0;

    bool mExec = false;
    uint8_t mExecFinishedThreshold = 0;
    bool mError = true;
    uint16_t mCyclesThisTick;
    bool mTicks = false;

    void command();

    public:
        SerialTracer(bool enabled = false) : 
            mPrint(PrintHelper(Serial)),
            mExec(enabled) {}

        virtual void exec(const EmuState &state, const Config &config);
        virtual void execFinished(const EmuState &state, const Config &config);
        virtual void error(ErrorType errorType, const EmuState &state, const Config &config); 
        virtual void tick(const EmuState &state, const Config &config); 
};

