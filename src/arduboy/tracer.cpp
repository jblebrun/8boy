#include "tracer.hpp"

void SerialTracer::exec(const EmuState &state, const Config &config) {
    if(Serial.peek() != -1) {
        mTracing = !mTracing;
    }
    if(!mTracing) {
        return;
    }
    mPrint.printfs(
        FS, F("0x"), X12, state.PC, ':', X16, state.Instruction, 

        FS, F(" ["),
        AX8, state.V, 16,
        FS, F("]  I="),
        X16, state.Index,
        
        FS, F(" shiftquirks="), X8, config.ShiftQuirk,
        '\r', '\n',
        DONE
    );
}

__FlashStringHelper* SerialTracer::errorMessage(ErrorType errorType) {
    switch(errorType) {
        case STACK_UNDERFLOW: return F("Stack Underflow!");
        case STACK_OVERFLOW: return F("Stack Overflow!");
        case OUT_OF_MEMORY: return F("Out of Memory!");
        case BAD_READ: return F("Couldn't Read Address!");
        case BAD_FETCH: return F("Instruction Fetch Failed!");
        case UNIMPLEMENTED_INSTRUCTION: return F("Unimplemented Instruction!");
    }
    return NULL;
}

void SerialTracer::error(ErrorType errorType, const EmuState &state, const Config &config) {
    __FlashStringHelper* errorMsg = errorMessage(errorType);
    if(errorMsg != NULL) {
        mPrint.printfs(FS, errorMsg, '\n', DONE);
    }
    for(int i = state.StackPointer - 1; i >= 0; i --) {
        mPrint.printfs(
            D8, i, 
            ':',
            X16, state.Stack[i], '\n',
            DONE
        );
    }
}


