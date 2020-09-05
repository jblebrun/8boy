#pragma once

#include "render.hpp"
#include "memory.hpp"
#include "errors.hpp"
#include "chip8-reg.hpp"

#include <stdint.h>


class Chip8 {
    // Rendering implementation from platform.
    Render &mRender;

    // Error handling implementation from platform.
    Errors &mErrors;
    
    // Memory implementation from platform.
    Memory &mMemory;

    // PC starts to 200 conventionally
    // we handle the offset into program memory when reading PC
    uint16_t mPC = 0x200;

    // Index register
    uint16_t mI = 0;

    // Default register set
    uint8_t mV[16];
    
    // Super Chip8 "RPL" register set.
    uint8_t mR[8];

    // Delay timer.
    uint16_t mDT;

    // Stack pointer for calls.
    uint8_t mSP = 0;

    // Stack for calls.
    uint16_t mStack[16];

    // True while the program is executing.
    bool mRunning = false;

    // Last button mask received from the platform.
    uint16_t mButtons = 0;

    // If mAwaitingKey is true, this holds the target register to store the
    // next keypress.
    uint8_t mWaitKeyDest = 0;

    // Indicates that we are paused waiting for the next keypress.
    bool mAwaitingKey = false;
    
    // Memory helpers.
    uint8_t readMem(uint16_t);
    void writeMem(uint16_t, uint8_t);

    // read buttons and handle any updates
    inline void handleButtons();

    // read an instruction
    inline uint16_t readInst();

    // execute a single fetched chip8 instruction
    void exec(uint16_t inst);

    // Instruction groups.
    
    // 0x0XXX - System (see submethods below).
    inline void groupSys(uint16_t);

    // 0x1nnn jump (no submethods).
    inline void groupJump(uint16_t);
    
    // 02nnn call (no submethods).
    inline void groupCall(uint16_t);
    
    // 0x3Xnn skip if equal immediate (no submethods).
    inline void groupSeImm(uint16_t);
    
    // 0x4Xnn skip if not equal immediate (no submethods).
    inline void groupSneImm(uint16_t);
    
    // 0x5XY0 skip if two registers hold equal values
    inline void groupSeReg(uint16_t);

    // 0x6Xnn - Load immediate
    inline void groupLdImm(uint16_t);

    // 0x7Xnn - add immediate
    inline void groupAddImm(uint16_t);

    // 0x8XYx - ALU Group
    inline void groupALU(uint16_t);

    // 0x9XYx   Skip if two registers hold inequal values
    inline void groupSneReg(uint16_t);

    //0xAnnn   load index immediate
    inline void groupLdiImm(uint16_t);

    // 0xBnnn   jump to I + xxx
    inline void groupJpV0Index(uint16_t);

    //0xCXnn   random, with mask.
    inline void groupRand(uint16_t);
    
    //0xDXYL   draw! If you think there's a bug in here, you're probably right.
    inline void groupGraphics(uint16_t);

    // 0xEX9E / 0xEXA1 - skip if key pressed/not pressed
    inline void groupKeyboard(uint16_t);

    // 0xFnnn - Load to various internal registers
    inline void groupLoad(uint16_t);


    // Instruction Sub-Methods
    
    // System group 0x0xxx
    // 0x00EE - Return from the most recently called subroutine.
    inline void ret();
    // 0x00FD - Exit. Stops execution and triggers the halt message on the
    // display.
    inline void exit();
    // 0x00FE/0x00FF - Enabled/Disable SChip8 hires mode.
    inline void setSuperhires(bool);

    // ALU group 0x8xxx

    // 0x8XY0   VX = Vy
    inline void aluLd(uint8_t x, uint8_t y);

    // 0x8XY2   VX = VX AND XY
    inline void aluOr(uint8_t x, uint8_t y);
    
    // 0x8XY1   VX = VX OR VY
    inline void aluAnd(uint8_t x, uint8_t y);

    // 0x8XY3   VX = VX XOR XY
    inline void aluXor(uint8_t x, uint8_t y);

    // 0x8XY4   VX = VX + VY, VF = carry
    inline void aluAdd(uint8_t x, uint8_t y);

    // 0x8XY5   VX = VX - VY, VF = 1 if borrow did not occur
    inline void aluSub(uint8_t x, uint8_t y);
    
    // 0x8XY6   VX = VX SHR VY, VF = bit shifted out
    inline void aluShr(uint8_t x, uint8_t y);

    // 0x8XY7   VX = VY - VX, VF = 1 if borrow did not occur
    inline void aluSubn(uint8_t x, uint8_t y);

    // 0x8XY8   VX = VX SHL VY, VF = bit shifted out
    inline void aluShl(uint8_t x, uint8_t y);


    // Load group 0xFxxx

    // 0xFX07 - Read delay timer into VX.
    inline void readDT(uint8_t);

    // 0xFX0A - Pause execution until a key is pressed.
    inline void waitK(uint8_t);

    // 0xFX15 - Set the delay timer to value in VX.
    inline void setDT(uint8_t);

    // 0xFX18 - Beep for the duration in VX.
    inline void makeBeep(uint16_t);

    // 0xFX1E - Add VX to I
    inline void addI(uint8_t);

    // 0xFX29 - Load low-res font character in VX
    inline void ldiFont(uint8_t);

    // 0xFX30 - Load hi-res font character in VX
    inline void ldiHiFont(uint8_t);

    // 0xFX33 - Write binary coded decimal encoding of VX to memory pointed to by I.
    inline void writeBCD(uint8_t);

    // 0xFX55 - Store V0-VX starting at I.
    inline void strReg(uint8_t);

    // 0xFX65 - Read into V0-VX starting at I.
    inline void ldReg(uint8_t);

    // 0xFX75 - Store registers into special platform storage
    // For example, "RPL" registers that were used in the original
    // SChip8 implementation.
    inline void strR(uint8_t);

    // 0xFX85 - Read registers from special platform storage
    // For example, "RPL" registers that were used in the original
    // SChip8 implementation.
    inline void ldR(uint8_t);


    public:
        // create a new Chip8 emulator with the provided renderer and memory
        // implementations.
        Chip8(Render &render, Errors &errors, Memory &memory);

        // Reset all registers and flags for the emulator instance, clear the memory,
        // and begin running.
        void Reset();

        // Read and execute one Chip8 operation. Instructions will be read from memory
        // using the provided memory implementation.
        // Returns false if emulator is no longer running and needs a Reset.
        bool Step();

        // Accept a bitmask of buttons that are pressed. The value will update the
        // internal button state of the emulator. If the emulator is waiting for a
        // keypress, that state will be detected here, and execution will continue.
        void Buttons(uint16_t buttons);

        // Updates any state that gets updated at 60Hz by chip-8
        // namely, beep timer and delay timer. Also triggers screen draw.
        void Tick();

       // returns true if the emulator is running
       bool Running() { return mRunning; }

       // if the emulator is stopped, start it, and vice-versa
       void Toggle() { mRunning = !mRunning; };

};
