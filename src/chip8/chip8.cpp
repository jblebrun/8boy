#include "chip8.hpp"
#include "render.hpp"
#include "string.h"
#include <stdio.h>

Chip8::Chip8(
    Render &render, 
    Memory &mem, 
    Tracer &tracer
) : mRender(render), mMemory(mem), mTracer(tracer) { }

// Reset all registers and flags for the emulator instance, clear the memory, and begin running.
void Chip8::Reset() {
    mState = EmuState();
    mState.Running = true;
    mRender.clear();
    mRender.setMode(CHIP8);
    mRender.beep(0);
}

// Tick updates any state that gets updated at 60Hz by chip-8
// namely, beep timer and delay timer, and triggers screen draw.
void Chip8::Tick() {
    handleButtons();
    mRender.render();
    mTracer.tick(mState, mConfig);
    if(mState.DelayTimer > 0) {
        mState.DelayTimer--;
    }
}

// Read the button state from the platform provider.
// If The emulator isn't running because it's waiting for a key, if the key is
// now pressed, it will be stored in the provided register.
inline void Chip8::handleButtons() {
    // Get platform buttons into the emulator state.
    mState.Buttons = mRender.buttons();
    
    // Handle the 0xFX0A (waitKey) instruction if needed.
    if (mState.AwaitingKey && mState.Buttons)  {
        // Find the first pressed button, lower hex value gets priority.
        uint8_t pressed = 0;
        while((mState.Buttons & 0x01) == 0) {
            mState.Buttons >>= 1;
            pressed++;
        }

        // Place the pressed button into the destinstaion resgister and
        // reset waiting flag.
        mState.V[mState.WaitKeyDest] = pressed;
        mState.AwaitingKey = false;
    }
}

// Expose the internal readWord inline function.
bool Chip8::ReadWord(uint16_t addr, uint16_t &result) {
    return readWord(addr, result);
}

// Read a 16-bit word from the memory of this Chip8 emulator at the
// specified address, handling endian byte swap.
inline bool Chip8::readWord(uint16_t addr, uint16_t &result) {
    if(!mMemory.read(addr, (uint8_t*)&result, 2)) return false;
    result = (result >> 8) | (result << 8);
    return true;
}

// Read and execute one Chip8 operation. Instructions will be read from memory
// using the provided memory implementation.
// If an ErrorType other than NO_ERROR is returned, then the PC will be pointing 
// to the last instruction executed.
ErrorType Chip8::Step() {
    // Let the caller know that we're not running.
    if(!mState.Running) return STOPPED;

    // We won't run any instructions while awaiting keys, but still considered
    // in the running state.
    if(mState.AwaitingKey) return NO_ERROR;

    mState.PC = mState.NextPC;

    if(!ReadWord(mState.PC, mState.Instruction)) {
        mState.Running = false;
        return BAD_FETCH;
    }

    // Increment PC now, none of the instructions depend on its value. 
    // This way, we don't need to keep track of whether or not the 
    // instruction resulted in a jump.
    mState.NextPC += 2;

    mTracer.exec(mState, mConfig);
    ErrorType error = exec(mState.Instruction);
    mTracer.execFinished(mState, mConfig);
    if(error != NO_ERROR) {
        mState.Running = false;
        mTracer.error(error, mState, mConfig);
    }
    return error;
}

inline ErrorType Chip8::exec(uint16_t inst) {
    // Dispatch to the instruction group based on the top nybble.
    switch (inst >> 12) {
        case 0x0: return groupSys(inst);
        case 0x1: groupJump(inst); break;
        case 0x2: return groupCall(inst);
        case 0x3: groupSeImm(inst); break;
        case 0x4: groupSneImm(inst); break;
        case 0x5: groupSeReg(inst); break;
        case 0x6: groupLdImm(inst); break;
        case 0x7: groupAddImm(inst); break;
        case 0x8: return groupALU(inst);
        case 0x9: groupSneReg(inst); break;
        case 0xA: groupLdiImm(inst); break;
        case 0xB: groupJpV0Index(inst); break;
        case 0xC: groupRand(inst); break;
        case 0xD: return groupGraphics(inst);
        case 0xE: return groupKeyboard(inst);
        case 0xF: return groupLoad(inst);
    }
    return NO_ERROR;
}

// 0x0XXX - System
inline ErrorType Chip8::groupSys(uint16_t inst) {
    switch(inst >> 4) {
        case 0x00C: mRender.scrollDown(inst&0x000F); break;
        default: switch(inst) {
            case 0x00E0: mRender.clear(); break;
            case 0x00EE: return ret();
            case 0x00FB: mRender.scrollRight(); break; // SCHIP8
            case 0x00FC: mRender.scrollLeft(); break;  // SCHIP8
            case 0x00FD: mState.Running = false; return STOPPED;
            case 0x00FE: setSuperhires(false); break;
            case 0x00FF: setSuperhires(true); break;
            case 0x0230: mRender.clear(); break; // Hi-Res variant
            default: return UNIMPLEMENTED_INSTRUCTION; 
        }
    }
    return NO_ERROR;
}

// 0x00EE - Return from the most recently called subroutine.
inline ErrorType Chip8::ret() {
    if(mState.StackPointer == 0) {
        return STACK_UNDERFLOW;
    }
    mState.StackPointer--;
    mState.NextPC = mState.Stack[mState.StackPointer];
    return NO_ERROR;
}

// 0x00FE/0x00FF - Enabled/Disable SChip8 hires mode.
inline void Chip8::setSuperhires(bool enabled) {
    mRender.setMode(enabled ? SCHIP8 : CHIP8);
}

// 0x1nnn jump
inline void Chip8::groupJump(uint16_t inst) {
    // Hack for handling original 64x64 Hi-Res mode
    // HiRes ML routines were at 200-248, hi-res
    // programs started at 0x2c0. They always start
    // with a jump to 0x0260, which runs a setup routine
    // in chip8/native asm:
    // * 6012 61be a200 f155
    //   Rewrite the first chip8 instruction to be a jump to 0x12be
    // * A series of reads of 0x200 space followed by writes to 0x000 
    //   space, presumably copying some native 1802 code contained in
    //   the Chip8 rom into the emulator space.
    // * It ends with 0x02ac at program address 0x02ac, 
    //   which jumps to a native RCA1802 routine at that location. 
    //   That routine eventually returns to running the chip8
    //   program. So in the emulator. 
    if(mState.PC == 0x200 && inst == 0x1260) {
        // Do what the ML code would do. 
        mRender.setMode(CHIP8HI);
        // Re-enter 0x0200, which is just going to jump to 0x02be.
        mState.NextPC = 0x02c0;
    } else {
        mState.NextPC = imm12(inst);
    }
}

// 02nnn call
inline ErrorType Chip8::groupCall(uint16_t inst) {
    // What does original interpreter do?
    if(mState.StackPointer >= 16) {
        return STACK_OVERFLOW;
    }
    mState.Stack[mState.StackPointer++] = mState.NextPC;
    mState.NextPC = imm12(inst);
    return NO_ERROR;
}

// 0x3Xnn skip if equal immediate
inline void Chip8::groupSeImm(uint16_t inst) {
    if(mState.V[x(inst)] == imm8(inst)) {
        mState.NextPC+=2;
    }
}

// 0x4Xnn skip if not equal immediate
inline void Chip8::groupSneImm(uint16_t inst) {
    if(mState.V[x(inst)] != imm8(inst)) {
        mState.NextPC+=2;
    }
}

// 0x5XY0 skip if two registers hold equal values
inline void Chip8::groupSeReg(uint16_t inst) {
    if(mState.V[x(inst)] == mState.V[y(inst)]) {
        mState.NextPC+=2;
    }
}

// 0x6Xnn - Load immediate
inline void Chip8::groupLdImm(uint16_t inst) {
    mState.V[x(inst)] = (uint8_t)inst;
}

// 0x7Xnn - add immediate
inline void Chip8::groupAddImm(uint16_t inst) {
    mState.V[x(inst)] += imm8(inst);
    // no VF carry indication? The documentation doesn't specify any.
}

// 0x8XYx - ALU Group
ErrorType Chip8::groupALU(uint16_t inst) {
    switch (inst&0xF) {
        case 0x0: aluLd(x(inst), y(inst)); break;
        case 0x1: aluOr(x(inst), y(inst)); break;
        case 0x2: aluAnd(x(inst), y(inst)); break;
        case 0x3: aluXor(x(inst), y(inst)); break;
        case 0x4: aluAdd(x(inst), y(inst)); break;
        case 0x5: aluSub(x(inst), y(inst)); break;
        case 0x6: aluShr(x(inst), y(inst)); break;
        case 0x7: aluSubn(x(inst), y(inst)); break;
        case 0xE: aluShl(x(inst), y(inst)); break;
        default: return UNIMPLEMENTED_INSTRUCTION;
    }
    return NO_ERROR;
}

// 0x8XY0   VX = Vy
inline void Chip8::aluLd(uint8_t x, uint8_t y) { mState.V[x] = mState.V[y]; }

// 0x8XY1   VX = VX OR VY
inline void Chip8::aluOr(uint8_t x, uint8_t y) { mState.V[x] = mState.V[x] | mState.V[y]; }

// 0x8XY2   VX = VX AND XY
inline void Chip8::aluAnd(uint8_t x, uint8_t y) { mState.V[x] = mState.V[x] & mState.V[y]; }

// 0x8XY3   VX = VX XOR XY
inline void Chip8::aluXor(uint8_t x, uint8_t y) { mState.V[x] = mState.V[x] ^ mState.V[y]; }

// 0x8XY4   VX = VX + VY, VF = carry
inline void Chip8::aluAdd(uint8_t x, uint8_t y) { 
    uint16_t result_carry = mState.V[x] + mState.V[y];
    mState.V[x] = mState.V[x] + mState.V[y]; 
    mState.V[0xF] = result_carry > 0x00FF ? 1 : 0;
}

// 0x8XY5   VX = VX - VY, VF = 1 if borrow did not occur
inline void Chip8::aluSub(uint8_t x, uint8_t y) { 
    // NOTE: some references indicate that VF = 1 if X > y. But it's X >= Y.
    // That is, VF = 1 if subtraction did not result in a borrow.
    uint8_t vf = mState.V[x] >= mState.V[y]; 
    mState.V[x] = mState.V[x] - mState.V[y];
    mState.V[0xF] = vf;
}

// 0x8XY6   VX = VX SHR VY, VF = bit shifted out
inline void Chip8::aluShr(uint8_t x, uint8_t y) { 
    uint8_t src = mState.V[mConfig.ShiftQuirk ? x : y];
    // Capture vf, but set actual VF register last.
    uint8_t vf = src&0x01 ? 1 : 0;
    mState.V[x] = src >> 1;
    mState.V[0xF] = vf; 
}

// 0x8XY7   VX = VY - VX, VF = 1 if borrow did not occur
inline void Chip8::aluSubn(uint8_t x, uint8_t y) { 
    // NOTE: some references indicate that VF = 1 if X > y. But it's X >= Y.
    // That is, VF = 1 if subtraction did not result in a borrow.
    uint8_t vf = mState.V[y] >= mState.V[x]; 
    mState.V[x] = mState.V[y] - mState.V[x];
    mState.V[0xF] = vf;
}

// 0x8XY8   VX = VX SHL VY, VF = bit shifted out
inline void Chip8::aluShl(uint8_t x, uint8_t y) { 
    uint8_t src = mState.V[mConfig.ShiftQuirk ? x : y];
    uint8_t vf = src&0x80 ? 1 : 0;
    mState.V[x] = src  << 1;
    mState.V[0xF] = vf;
}

// 0x9XYx   Skip if two registers hold inequal values
void Chip8::groupSneReg(uint16_t inst) {
    if(mState.V[x(inst)] != mState.V[y(inst)]) {
        mState.NextPC+=2;
    }
}

//0xAnnn   load index immediate
void Chip8::groupLdiImm(uint16_t inst) {
    mState.Index = imm12(inst);
}

// 0xBnnn   jump to v[0] + xxx
void Chip8::groupJpV0Index(uint16_t inst) {
    mState.NextPC = mState.V[0]+imm12(inst);
}

//0xCXnn   random, with mask.
void Chip8::groupRand(uint16_t inst) {
    mState.V[x(inst)] = mRender.random() & imm8(inst);
}

//0xDXYL   draw! If you think there's a bug in here, you're probably right.
ErrorType Chip8::groupGraphics(uint16_t inst) {
    // The number of lines in the sprite that we should draw.
    uint8_t rows = imm4(inst);
    // X, Y location of the sprite, from registers.
    uint8_t xc = mState.V[x(inst)];
    uint8_t yc = mState.V[y(inst)];

    // In chip8 mode, we draw 8 columns. 
    uint8_t cols = 8;
    // We will be shifting the data left and masking out the 8th MSB
    uint16_t mask = 0x80;

    // In super hires mode, drawing with rows == 0 triggers 16x16 sprite mode.
    bool superSprite = rows == 0 && mRender.mode() == SCHIP8;

    if(superSprite) {
       rows = 16;
        // We will be shifting the data left and masking out the 16th MSB.
       mask = 0x8000;
       cols = 16;
    }

    // Clear the collision flag.
    mState.V[0xF] = 0;

    // Draw row-by-row
    for(int row = 0; row < rows; row++) {
        // Collect the data to draw from memory.
        uint16_t rowData;
        if(superSprite) {
            if(!ReadWord(mState.Index+row*2, rowData)) return BAD_READ;
        } else {
            if(!mMemory.read(mState.Index+row, (uint8_t*)&rowData, 1)) return BAD_READ;
        }

        for(int col = 0; col < cols; col++) {
            bool on = rowData&mask;
            uint8_t px = xc + col;
            uint8_t py = yc + row;

            // Draw the pixel
            mState.V[0xF] |= mRender.drawPixel(px,py, on);
            rowData<<=1;

        }
    }
    return NO_ERROR;
}

// 0xEX9E / 0xEXA1 - skip if key pressed/not pressed
ErrorType Chip8::groupKeyboard(uint16_t inst) {
    uint8_t key = mState.V[x(inst)];
    uint16_t mask = 0x01 << key;
    switch(imm8(inst)) {
        case 0x9E:
            if(mState.Buttons & mask) {
                mState.NextPC += 2;
            }
            break;
        case 0xA1:
            if(!(mState.Buttons & mask)) {
                mState.NextPC += 2;
            }
            break;
        default: return UNIMPLEMENTED_INSTRUCTION;
    }
    return NO_ERROR;
}

// 0xFnnn - Load to various internal registers
ErrorType Chip8::groupLoad(uint16_t inst) {
    switch(inst&0xFF) {
        case 0x07: readDT(x(inst)); break;
        case 0xA: waitK(x(inst)); break;
        case 0x15: setDT(x(inst)); break;
        case 0x18: makeBeep(x(inst)); break;
        case 0x1E: addI(x(inst)); break;
        case 0x29: ldiFont(x(inst)); break;
        case 0x30: ldiHiFont(x(inst)); break;
        case 0x33: return writeBCD(x(inst));
        case 0x55: return strReg(x(inst));
        case 0x65: return ldReg(x(inst));
        case 0x75: strR(x(inst)); break;
        case 0x85: ldR(x(inst)); break;
        default: return UNIMPLEMENTED_INSTRUCTION;
    }
    return NO_ERROR;
}

// 0xFX07 - Read delay timer into VX.
inline void Chip8::readDT(uint8_t into) { mState.V[into] = mState.DelayTimer; }

// 0xFX0A - Pause execution until a key is pressed.
inline void Chip8::waitK(uint8_t into) {
    mState.AwaitingKey = true;
    mState.WaitKeyDest = into;
}

// 0xFX15 - Set the delay timer to value in VX.
inline void Chip8::setDT(uint8_t from) { mState.DelayTimer = mState.V[from]; }

// 0xFX18 - Beep for the duration in VX.
inline void Chip8::makeBeep(uint16_t durReg) { 
    mRender.beep(mState.V[durReg]);
}

// 0xFX1E - Add VX to I
inline void Chip8::addI(uint8_t from) { mState.Index += mState.V[from]; }

// 0xFX29 - Load low-res font character in VX
inline void Chip8::ldiFont(uint8_t from) { mState.Index = 5 * mState.V[from]; }

// 0xFX30 - Load hi-res font character in VX. It appears immediately after the low-res font.
inline void Chip8::ldiHiFont(uint8_t from) { mState.Index = 0x10*5 + (10 * mState.V[from]); }

// 0xFX33 - Write binary coded decimal encoding of VX to memory pointed to by I.
inline ErrorType Chip8::writeBCD(uint8_t from) {
    uint8_t val = mState.V[from];
    uint8_t vals[3] = {(uint8_t)(val/100), (uint8_t)((val/10)%10), (uint8_t)(val%10)};
    return mMemory.write(mState.Index, vals, 3) ? NO_ERROR : OUT_OF_MEMORY;
}

// 0xFX55 - Store V0-VX starting at I.
inline ErrorType Chip8::strReg(uint8_t upto) {
    return mMemory.write(mState.Index, mState.V, upto+1) ? NO_ERROR : OUT_OF_MEMORY;
}

// 0xFX65 - Read into V0-VX starting at I.
inline ErrorType Chip8::ldReg(uint8_t upto) {
    return mMemory.read(mState.Index, mState.V, upto+1) ? NO_ERROR : BAD_READ;
}


// 0xFX75 - Store registers into special platform storage
inline void Chip8::strR(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        mState.R[i] = mState.V[i];
    }
}

// 0xFX85 - Read registers from special platform storage
inline void Chip8::ldR(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        mState.V[i] = mState.R[i];
    }
}
