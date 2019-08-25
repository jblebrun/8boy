#include <stdint.h>
#include <Arduboy2.h>


#define SLAB_SIZE 16
#define SLAB_COUNT 32

struct Slab {
    uint8_t page;
    uint8_t data[SLAB_SIZE];
};

class Chip8;

typedef void(Chip8::*groupFunc)(uint16_t);

class Chip8 {

    // Rendering
    Arduboy2 &mBoy;
    BeepPin1 beep;

    // Program info
    const uint8_t *mProgram;
    uint16_t mProgramSize;
    
    // registers
    // PC starts to 200 conventionally
    // we handle the offset when reading PC
    uint16_t mPC = 0x200;
    uint16_t mI = 0;
    uint8_t mV[16];
    uint16_t mDT;
    uint8_t mSP = 0;
    uint16_t mStack[16];

    // Other state
    bool mHires = false;
    bool mRunning = false;
    uint16_t mButtons = 0;
    bool mWaitKey;
    
    void run();
    inline void halt();
    inline void unimpl(uint16_t);

    // Memory
    Slab mSlabs[SLAB_COUNT];
    uint8_t readMem(uint16_t);
    void writeMem(uint16_t, uint8_t);
    Slab* findSlab(uint16_t);

    
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
    inline void scrollLeft();
    inline void scrollRight();
    inline void exit();
    inline void setHires(bool);

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


    public:
       Chip8(Arduboy2 &boy);
       void Load(const uint8_t program[], uint16_t size);
       void Reset();
       void Step();
       void Buttons(uint16_t buttons);
       bool Running() { return mRunning; }
       void Toggle() { mRunning = !mRunning; };

};
