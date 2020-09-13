#include "../src/chip8/tracer.hpp"

class ConsoleTracer : public Tracer {
    virtual void exec(const EmuState &state) {
        printf("%4X:%4X\n", state.PC, state.Instruction);
    }

    const char* errorMsg(ErrorType errorType) {
        switch(errorType) {
            case STACK_UNDERFLOW: return "Stack Underflow!";
            case STACK_OVERFLOW: return "Stack Overflow!";
            case OUT_OF_MEMORY: return "Out of Memory!";
            case BAD_READ: return "Couldn't Read Address!";
            case BAD_FETCH: return "Instruction Fetch Failed!";
            case UNIMPLEMENTED_INSTRUCTION: return "Unimplemented Instruction!";
            default: return "Program Halted.";
        }
    }

    virtual void error(ErrorType errorType, const EmuState &state) {
        printf("%s\n", errorMsg(errorType));
    }
};
