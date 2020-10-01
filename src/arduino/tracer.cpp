#include "tracer.hpp"


inline void SerialTracer::command() {
    if(Serial.peek() != -1) {
        uint8_t com = Serial.read();
        switch(com) {
            case ' ': mExec = !mExec; break;
            case 't': mTicks = !mTicks; break;
            case '0': mExecFinishedThreshold = 0; break;
            case '1': mExecFinishedThreshold = 1; break;
            case '2': mExecFinishedThreshold = 2; break;
            case '3': mExecFinishedThreshold = 3; break;
        }
    }
}

void SerialTracer::exec(const EmuState &state, const Config &config) {
    mCyclesThisTick++;
    command();
    if(mExec) {
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
    mExecStart = micros();
}

void SerialTracer::execFinished(const EmuState &state, const Config &config) {
    command();
    uint16_t time = micros() - mExecStart;
    if(mExecFinishedThreshold == 1 ||
           (mExecFinishedThreshold == 2 && time > 30) ||
            (mExecFinishedThreshold == 3 && time > 100)) { 
        mPrint.printfs(
            X16, state.Instruction, 
            ' ',
            D16, time,
            FS, F("us\r\n"),
            DONE
        );
    }
}

void SerialTracer::tick(const EmuState &state, const Config &config) {
    command();
    if(mTicks) {
        mPrint.printfs(
            D16, mCyclesThisTick,
            FS, F(" cycles/tick\r\n"),
            DONE
        );
        mCyclesThisTick = 0;
    }
}


const __FlashStringHelper* SerialTracer::errorMessage(ErrorType errorType) {
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
    const __FlashStringHelper* errorMsg = errorMessage(errorType);
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


