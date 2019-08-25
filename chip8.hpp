#include <stdint.h>
#include <Arduboy2.h>


class Chip8;

typedef void(Chip8::*groupFunc)(uint16_t);

class Chip8 {
    const uint8_t *mProgram;
    uint16_t mProgramSize;
    void run();
    
    // registers
    // PC starts to 200 conventionally
    // we handle the offset when reading PC
    uint16_t mPC = 0x200;
    uint16_t mI = 0;
    uint8_t mV[16];
    uint16_t mStack[16];
    uint8_t mSP = 0;
    uint16_t mDT;

    bool mHires = false;

    uint16_t mWrites = 0;

    bool mRunning = false;

#define MEM_SIZE 512
    uint8_t mM[MEM_SIZE];

#define MAX_CELLS 48
    uint8_t mCellIndex = 0;
    uint16_t mCellAddrs[MAX_CELLS];
    uint8_t mCellValues[MAX_CELLS];

    bool readCell(uint16_t, uint8_t*);
    void writeCell(uint16_t, uint8_t);

    Arduboy2 &mBoy;
    BeepPin1 beep;
    
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

    uint8_t readMem(uint16_t);
    void writeMem(uint16_t, uint8_t);

    inline void unimpl(uint16_t);

    uint16_t mButtons = 0;

    bool mWaitKey;

    void scrollLeft();
    void scrollRight();

    public:
       Chip8(Arduboy2 &boy);
       void Load(const uint8_t program[], uint16_t size);
       void Reset();
       void Step();
       void Buttons(uint16_t buttons);
       bool Running();
       void Toggle();

};
