#pragma once
#include <stdint.h>
#include <string.h>

struct EmuState {
    // Chip8 Registers
    
    // Program Counter for the currently executing instruction.
    uint16_t PC = 0x200;

    // Current executing instruction
    uint16_t Instruction = 0;

    // Program Counter for the next step.
    uint16_t NextPC = 0x200;

    // Index Register
    uint16_t Index = 0;

    // Delay Timer
    uint16_t DelayTimer = 0;

    // General-Purpose Registers V0-VF
    uint8_t V[16] = {0};

    // Special flag registers
    uint8_t R[8] = {0};

    // Call Stack (every CALL pushes the current PC onto this.
    uint16_t Stack[16] = {0};
    
    // Stack Pointer
    uint8_t StackPointer = 0;

    // ArduboyChip8 data
    // True if the emualtor is in the running state
    bool Running = false;

    // The current state of keys. 0-F, bit 0 to bit 15.
    uint16_t Buttons = 0;

    // If AwaitingKey is true, this is the register that will
    // result the key that's pressed.
    uint8_t WaitKeyDest = 0;

    // Set to true if the emulator is halted waiting for a keypress.
    bool AwaitingKey = false;
};
