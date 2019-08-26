#pragma once

#include <stdint.h>

class Render {
    public:
    virtual bool getPixel(uint8_t x, uint8_t y) = 0;
    virtual void drawPixel(uint8_t x, uint8_t y, bool on) = 0;
    virtual void scrollDown(uint8_t amt) = 0;
    virtual void scrollLeft() = 0;
    virtual void scrollRight() = 0;
    virtual void beep(uint8_t dur) = 0;
    virtual void render() = 0;
    virtual uint8_t random() = 0;
    virtual void clear() = 0;

    // errors
    virtual void stackUnderflow(uint16_t addr) = 0;
    virtual void stackOverflow(uint16_t addr) = 0;
    virtual void oom(uint16_t addr) = 0;
    virtual void badread(uint16_t addr) = 0;
    virtual void exit() = 0;
    virtual void unimpl(uint16_t addr, uint16_t inst) = 0;
};

class Memory {
    public: 
        virtual void load(const uint8_t *program, const uint16_t size) = 0;
        virtual bool read(uint16_t addr, uint8_t &val) = 0;
        virtual bool write(uint16_t addr, uint8_t val) = 0;
        virtual void reset() = 0;
};


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
       Chip8(Render &boy, Memory &memory);
       void Reset();
       void Step();
       void Buttons(uint16_t buttons);
       void Tick();
       bool Running() { return mRunning; }
       void Toggle() { mRunning = !mRunning; };

};
