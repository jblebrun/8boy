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

#define MAX_CELLS 16
    uint8_t mCellIndex = 0;
    uint16_t mCellAddrs[MAX_CELLS];
    uint8_t mCellValues[MAX_CELLS];

    bool readCell(uint16_t, uint8_t*);
    void writeCell(uint16_t, uint8_t);

    Arduboy2 &mBoy;
    BeepPin1 beep;

    
    // XXX MEMORY HANDLING --
    // if below size of program, use cell
    // otherwise, hope it starts right after program
    

    // function table
    groupFunc groupFuncs[16] {
        // 0xxx - System instructions
        &Chip8::groupSys,    

        // 1xxx - uncondition jump
        &Chip8::groupJump,

        // 2xxx - call
        &Chip8::groupCall,

        // 3xxx - skip if equal immediate
        &Chip8::groupSeImm,  

        // 4xxx - skipp if not equal immediate
        &Chip8::groupSneImm,

        // 5xxx - skip if equal reg
        &Chip8::groupSeReg, 

        // 6xxx - load immediate
        &Chip8::groupLdImm,

         // 7xxx - add immediate
        &Chip8::groupAddImm,

        // 8xxx - two-register ALU ops
        &Chip8::groupALU,

        // 9xxx - skip if no equal reg
        &Chip8::groupSneReg,

        // Axxx - load index register immediate
        &Chip8::groupLdiImm,

        // Bxxx - V0-indexed jump
        &Chip8::groupJpV0Index,

        // Cxxx - rand
        &Chip8::groupRand,

        // Dxxx - graphics
        &Chip8::groupGraphics,

        // Exxx - keyboard
        &Chip8::groupKeyboard,

        // Fxxx - single register load/add group
        &Chip8::groupLoad,
    };
    
    void groupSys(uint16_t);
    void groupJump(uint16_t);
    void groupCall(uint16_t);
    void groupSeImm(uint16_t);
    void groupSneImm(uint16_t);
    void groupSeReg(uint16_t);
    void groupLdImm(uint16_t);
    void groupAddImm(uint16_t);
    void groupALU(uint16_t);
    void groupSneReg(uint16_t);
    void groupLdiImm(uint16_t);
    void groupJpV0Index(uint16_t);
    void groupRand(uint16_t);
    void groupGraphics(uint16_t);
    void groupKeyboard(uint16_t);
    void groupLoad(uint16_t);

    uint8_t readMem(uint16_t);
    void writeMem(uint16_t, uint8_t);

    void unimpl(uint16_t);

    uint16_t lastinst;
    uint16_t mButtons = 0;

    bool mWaitKey;

    void scrollLeft();
    void scrollRight();

    public:
       Chip8(Arduboy2 &boy);
       void Load(const uint8_t program[], uint16_t size);
       void Reset();
       void Step();
       void Buttons(uint8_t buttons);
       bool Running();
       void Toggle();

};
