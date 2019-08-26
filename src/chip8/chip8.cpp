#include "chip8.hpp"

Chip8::Chip8(Render &render, Errors &errors, Memory &mem) : mRender(render), mErrors(errors), mMemory(mem) { }

void Chip8::Reset() {
    mPC = 0x200;
    mI = 0;
    mDT = 0;
    mSP = 0;
    mRunning = true;
    mHires = false;
    mSuperhires = false;
    mRender.clear();
    mRender.beep(0);
    mMemory.reset();
}

// Tick updates any state that gets updated at 60Hz by chip-8
// namely, beep timer and delay timer, and triggers screen draw.
void Chip8::Tick() {
    mRender.render();
    if(mDT > 0) {
        mDT--;
    }
}
    
void Chip8::Buttons(uint16_t buttons) {
    mButtons = buttons;
    if (mWaitKey && buttons) {
        mWaitKey = false;
        mRunning = true;
    }
}

void Chip8::Step() {
    // endian fix
    uint8_t hi = readMem(mPC);
    uint8_t lo = readMem(mPC+1);
    uint16_t inst = (uint16_t(hi << 8) | lo) ;

    if(mPC == 0x200 && inst == 0x1260) {
        mHires = true;
        inst = 0x12C0;
    }
    mPC+=2;
    switch (inst >> 12) {
        case 0x0: return groupSys(inst);
        case 0x1: return groupJump(inst);
        case 0x2: return groupCall(inst);
        case 0x3: return groupSeImm(inst);
        case 0x4: return groupSneImm(inst);
        case 0x5: return groupSeReg(inst);
        case 0x6: return groupLdImm(inst);
        case 0x7: return groupAddImm(inst);
        case 0x8: return groupALU(inst);
        case 0x9: return groupSneReg(inst);
        case 0xA: return groupLdiImm(inst);
        case 0xB: return groupJpV0Index(inst);
        case 0xC: return groupRand(inst);
        case 0xD: return groupGraphics(inst);
        case 0xE: return groupKeyboard(inst);
        case 0xF: return groupLoad(inst);
    }
}

inline void Chip8::cls() {
    mRender.clear();
}

inline void Chip8::ret() {
    if(mSP == 0) {
        mErrors.stackUnderflow(mPC-2);
        mRunning = false;
        return;
    }
    mSP--;
    mPC = mStack[mSP];
}

inline void Chip8::setSuperhires(bool enabled) {
    mSuperhires = enabled;
}

inline void Chip8::exit() {
    mRender.exit();
    mRunning = false;
}


inline void Chip8::groupSys(uint16_t inst) {
    switch(inst >> 4) {
        case 0x00C: return mRender.scrollDown(inst&0x000F);
        default: switch(inst) {
            case 0x00E0: return cls();
            case 0x00EE: return ret();
            case 0x00FB: return mRender.scrollRight();
            case 0x00FC: return mRender.scrollLeft();
            case 0x00FD: return exit();
            case 0x00FE: return setSuperhires(false);
            case 0x00FF: return setSuperhires(true);
            case 0x0230: return cls();
            default: mErrors.unimpl(mPC-2, inst);
        }
    }
}


// 0x1xxx jump
inline void Chip8::groupJump(uint16_t inst) {
    mPC = imm12(inst);
}

// 02xxx call
inline void Chip8::groupCall(uint16_t inst) {
    // What does original interpreter do?
    if(mSP >= 16) {
        mErrors.stackOverflow(mPC-2);
        return;
    }
    mStack[mSP++] = mPC;
    mPC = imm12(inst);
}

// 0x3000 skip if equal immediate
inline void Chip8::groupSeImm(uint16_t inst) {
    if(mV[x(inst)] == imm8(inst)) {
        mPC+=2;
    }
}

// 0x4xxxx skip if not equal immediate
inline void Chip8::groupSneImm(uint16_t inst) {
    if(mV[x(inst)] != imm8(inst)) {
        mPC+=2;
    }
}

// 0x5xxx skip if two registers hold equal values
inline void Chip8::groupSeReg(uint16_t inst) {
    if(mV[x(inst)] == mV[y(inst)]) {
        mPC+=2;
    }
}

//0x6xxx
inline void Chip8::groupLdImm(uint16_t inst) {
    mV[x(inst)] = (uint8_t)inst;
}

//0x7xxx add immediate
inline void Chip8::groupAddImm(uint16_t inst) {
    mV[x(inst)] += imm8(inst);
    // no carry?
}

//0x8xx0 - ALU
inline void Chip8::aluLd(uint8_t x, uint8_t y) { mV[x] = mV[y]; }
inline void Chip8::aluOr(uint8_t x, uint8_t y) { mV[x] = mV[x] | mV[y]; }
inline void Chip8::aluAnd(uint8_t x, uint8_t y) { mV[x] = mV[x] & mV[y]; }
inline void Chip8::aluXor(uint8_t x, uint8_t y) { mV[x] = mV[x] ^ mV[y]; }
inline void Chip8::aluAdd(uint8_t x, uint8_t y) { 
    uint16_t result_carry = mV[x] + mV[y];
    mV[x] = mV[x] + mV[y]; 
    mV[0xF] = result_carry > 0x00FF ? 1 : 0;
}
inline void Chip8::aluSub(uint8_t x, uint8_t y) { 
    // NOTE: many references are wrong!
    // Consult *original* Cosmac VIP manual
    mV[0xF] = mV[x] >= mV[y]; 
    mV[x] = mV[x] - mV[y];
}

inline void Chip8::aluSubn(uint8_t x, uint8_t y) { 
    // NOTE: many references are wrong!
    // Consult *original* Cosmac VIP manual
    mV[0xF] = mV[y] >= mV[x]; 
    mV[x] = mV[y] - mV[x];
}

inline void Chip8::aluShr(uint8_t x, uint8_t y) { 
    mV[0xF] = mV[x]&0x01 ? 1 : 0;
    mV[x] = mV[x] >> 1;
}

inline void Chip8::aluShl(uint8_t x, uint8_t y) { 
    mV[0xF] = mV[x]&0x80 ? 1 : 0;
    mV[x] = mV[x] << 1;
}

void Chip8::groupALU(uint16_t inst) {
    switch (inst&0xF) {
        case 0x0: return aluLd(x(inst), y(inst));
        case 0x1: return aluOr(x(inst), y(inst));
        case 0x2: return aluAnd(x(inst), y(inst));
        case 0x3: return aluXor(x(inst), y(inst));
        case 0x4: return aluAdd(x(inst), y(inst));
        case 0x5: return aluSub(x(inst), y(inst));
        case 0x6: return aluShr(x(inst), y(inst));
        case 0x7: return aluSubn(x(inst), y(inst));
        case 0xE: return aluShl(x(inst), y(inst));
        default:
            mErrors.unimpl(mPC-2, inst);
    }
}


// 0x9xxx - Skip if two registers hold inequal values
void Chip8::groupSneReg(uint16_t inst) {
    if(mV[x(inst)] != mV[y(inst)]) {
        mPC+=2;
    }
}

//0xAxxx load index immediate
void Chip8::groupLdiImm(uint16_t inst) {
    mI = imm12(inst);
}

// 0xBxxx jump to I + xxx
void Chip8::groupJpV0Index(uint16_t inst) {
    mErrors.unimpl(mPC-2, inst);
    //mPC = mI+imm12(inst);
}

//0xCxxx random
void Chip8::groupRand(uint16_t inst) {
    mV[x(inst)] = mRender.random() & imm8(inst);
}

//0xDxxx - draw
void Chip8::groupGraphics(uint16_t inst) {
    uint8_t rows = imm4(inst);
    uint8_t xc = mV[x(inst)];
    uint8_t yc = mV[y(inst)];
    uint8_t cols = 8;
    uint16_t mask = 0x80;
    if(rows == 0 && mSuperhires) {
       rows = 16;
       mask = 0x8000;
       cols = 16;
    }

    bool collide = false;

    uint8_t scalex = mSuperhires ? 1 : 2;
    uint8_t scaley = mSuperhires | mHires ? 1 : 2;
    mV[0xF] = 0;
    for(int row = 0; row < rows; row++) {
        uint16_t rowData;
        if(mSuperhires && rows == 16) {
            rowData = readMem(mI+row*2);
            rowData <<= 8;
            rowData |= readMem(mI+row*2+1);
        } else {
            rowData = readMem(mI+row);
        }
        for(int col = 0; col < cols; col++) {
            bool on = rowData&mask;
            uint8_t py = scaley*((yc+row)%(64/scaley));
            uint8_t px = scalex*((xc+col)%(128/scalex));

            bool wasOn = mRender.getPixel(px, py);
            uint8_t newColor = wasOn ^ on;
            mV[0xF] |= (wasOn & on) ? 1 : 0;
            mRender.drawPixel(px,py, newColor);
            if(!mSuperhires) {
                mRender.drawPixel(px+1,py, newColor);
                if(!mHires) {
                    mRender.drawPixel(px+1,py+1, newColor);
                    mRender.drawPixel(px,py+1, newColor);
                }
            }
            rowData<<=1;

        }
    }
}

// 0xExxx XXX -keyboard to keys
void Chip8::groupKeyboard(uint16_t inst) {
    uint8_t key = mV[x(inst)];
    uint16_t mask = 0x01 << key;
    switch(imm8(inst)) {
        case 0x9E:
            if(mButtons & mask) {
                mPC += 2;
            }
            break;
        case 0xA1:
            if(!(mButtons & mask)) {
                mPC += 2;
            }
            break;
        default:
            mErrors.unimpl(mPC-2, inst);
    }
}


inline void Chip8::readDT(uint8_t into) { mV[into] = mDT; }
inline void Chip8::waitK(uint8_t into) {
    mRunning = false;
    mWaitKey = true;
}
inline void Chip8::setDT(uint8_t from) { mDT = mV[from]; }
inline void Chip8::makeBeep(uint16_t dur) { 
    mRender.beep(dur);
}
inline void Chip8::addI(uint8_t from) { mI += mV[from]; }
inline void Chip8::ldiFont(uint8_t from) { mI = 5 * mV[from]; }
inline void Chip8::ldiHiFont(uint8_t from) { mI = 10 * mV[from]; }
inline void Chip8::writeBCD(uint8_t from) {
    uint8_t val = mV[from];
    writeMem(mI, val/100);
    writeMem(mI+1, (val/10)%10);
    writeMem(mI+2, val%10);
}
inline void Chip8::strReg(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        writeMem(mI+i, mV[i]);
    }
}

inline void Chip8::ldReg(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        mV[i] = readMem(mI+i);
    }
}

inline void Chip8::strR(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        mR[i] = mV[i];
    }
}

inline void Chip8::ldR(uint8_t upto) {
    for(int i = 0; i <= upto; i++) {
        mV[i] = mR[i];
    }
}



void Chip8::groupLoad(uint16_t inst) {
    switch(inst&0xFF) {
        case 0x07: return readDT(x(inst));
        case 0xA: return waitK(x(inst));
        case 0x15: return setDT(x(inst));
        case 0x18: return makeBeep(imm8(inst));
        case 0x1E: return addI(x(inst));
        case 0x29: return ldiFont(x(inst));
        case 0x30: return ldiHiFont(x(inst));
        case 0x33: return writeBCD(x(inst));
        case 0x55: return strReg(x(inst));
        case 0x65: return ldReg(x(inst));
        case 0x75: return strR(x(inst));
        case 0x85: return ldR(x(inst));
        default:
            mErrors.unimpl(mPC-2, inst);
    }
}

inline void Chip8::writeMem(uint16_t addr, uint8_t val) {
    if(!mMemory.write(addr, val)) {
        mErrors.oom(mPC-2);
        mRunning = false;
    }
}

inline uint8_t Chip8::readMem(uint16_t addr) {
    uint8_t val;
    if(!mMemory.read(addr, val)) {
        mErrors.badread(mPC-2);
        mRunning = false;
    }
    return val;
}


