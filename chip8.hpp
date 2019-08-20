#include <stdint.h>
#include <Arduboy2.h>


class Chip8;

typedef void(Chip8::*groupFunc)(uint16_t);

class Chip8 {
    

    const uint8_t *mProgram;
    void run();
    
    // registers
    // PC starts to 200 conventionally
    // we handle the offset when reading PC
    uint16_t mPC = 0x200;
    uint16_t mI = 0;
    uint16_t mV[16];
    uint16_t mStack[16];
    uint8_t mSP = 0;
    uint16_t mDT;

    uint16_t mWrites = 0;

    bool mRunning = false;

    uint8_t mM[512];

    Arduboy2 &mBoy;

    
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

    void unimpl(uint16_t);

    uint16_t lastinst;

    public:
       Chip8(Arduboy2 &boy);
       void Load(const uint8_t program[]);
       void Reset();
       void Step();
       bool Running();
       void Toggle();

};
