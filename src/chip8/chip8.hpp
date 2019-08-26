#pragma once

#include "render.hpp"
#include "memory.hpp"
#include "chip8-reg.hpp"

#include <stdint.h>


class Chip8 {
    Render &mRender;

    // registers
    // PC starts to 200 conventionally
    // we handle the offset when reading PC
    uint16_t mPC = 0x200;
    uint16_t mI = 0;
    uint8_t mV[16];
    uint8_t mR[8];
    uint16_t mDT;
    uint8_t mSP = 0;
    uint16_t mStack[16];
    Memory &mMemory;

    // Other state
    bool mHires = false;
    bool mSuperhires = false;
    bool mRunning = false;
    uint16_t mButtons = 0;
    bool mWaitKey;
    
    // Memory
    uint8_t readMem(uint16_t);
    void writeMem(uint16_t, uint8_t);

    
    // instruction handlers
    // Instruction groups
    inline void groupSys(uint16_t);
    inline void groupJump(uint16_t);
    inline void groupCall(uint16_t);
    inline void groupSeImm(uint16_t);
    inline void groupSneImm(uint16_t);
    inline void groupSeReg(uint16_t);
    inline void groupLdImm(uint16_t);
    inline void groupAddImm(uint16_t);
    inline void groupALU(uint16_t);
    inline void groupSneReg(uint16_t);
    inline void groupLdiImm(uint16_t);
    inline void groupJpV0Index(uint16_t);
    inline void groupRand(uint16_t);
    inline void groupGraphics(uint16_t);
    inline void groupKeyboard(uint16_t);
    inline void groupLoad(uint16_t);

    // System group 0x0xxx
    inline void cls();
    inline void ret();
    inline void scrollDown(uint8_t amt);
    inline void scrollLeft();
    inline void scrollRight();
    inline void exit();
    inline void setSuperhires(bool);

    // ALU group 0x8xxx
    inline void aluLd(uint8_t x, uint8_t y);
    inline void aluAnd(uint8_t x, uint8_t y);
    inline void aluOr(uint8_t x, uint8_t y);
    inline void aluXor(uint8_t x, uint8_t y);
    inline void aluAdd(uint8_t x, uint8_t y);
    inline void aluSub(uint8_t x, uint8_t y);
    inline void aluSubn(uint8_t x, uint8_t y);
    inline void aluShl(uint8_t x, uint8_t y);
    inline void aluShr(uint8_t x, uint8_t y);

    // Load group 0xFxxx
    inline void readDT(uint8_t);
    inline void waitK(uint8_t);
    inline void setDT(uint8_t);
    inline void makeBeep(uint16_t);
    inline void addI(uint8_t);
    inline void ldiFont(uint8_t);
    inline void ldiHiFont(uint8_t);
    inline void writeBCD(uint8_t);
    inline void strReg(uint8_t);
    inline void ldReg(uint8_t);
    inline void strR(uint8_t);
    inline void ldR(uint8_t);


    public:
       // create a new Chip8 emulator with the provided renderer and memory implementations.
       Chip8(Render &render, Memory &memory);

       // reset the state of the emulator (clear internal registers, screen, etc.
       void Reset();

       // execute one instruction
       void Step();

       // update the state of the buttons (masked, one bit for each key 0-F)
       void Buttons(uint16_t buttons);

       // update the 60Hz delay timer
       void Tick();

       // returns true if the emulator is running
       bool Running() { return mRunning; }

       // if the emulator is stopped, start it, and vice-versa
       void Toggle() { mRunning = !mRunning; };

};
