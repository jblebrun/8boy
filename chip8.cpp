#include "chip8.hpp"
#include <Arduino.h>


const uint8_t font[] PROGMEM = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

const uint8_t fonthi[] PROGMEM = {
    0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

Chip8::Chip8(Arduboy2 &boy) : mBoy(boy) {
    beep.begin();
}

void Chip8::Load(const uint8_t *program, const uint16_t size) {
    mProgram = program;
    mProgramSize = size;
}

void Chip8::Reset() {
    mPC = 0x200;
    mSP = 0;
    mRunning = true;
    mBoy.clear();
    mHires = false;
    mCellIndex = 0;
}

bool Chip8::Running() {
    return mRunning;
}

void Chip8::Toggle() {
    mRunning = !mRunning;
}

void Chip8::Buttons(uint16_t buttons) {
    beep.timer();
    if(mDT > 0) {
        mDT--;
    }
    
    mButtons = buttons;
    if (mWaitKey && buttons) {
        mWaitKey = false;
        halt();
    }
}

void Chip8::Step() {
    // endian fix
    uint8_t hi = pgm_read_byte(&mProgram[mPC-0x200]);
    uint8_t lo = pgm_read_byte(&mProgram[mPC+1-0x200]);
    uint16_t inst = (uint16_t(hi << 8) | lo) ;

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

// inst read helpers
inline uint8_t x(uint16_t inst) { return (inst>>8)&0x0F; }
inline uint8_t y(uint16_t inst) { return (inst>>4)&0x0F; }
inline uint8_t imm4(uint16_t inst) { return inst&0x0F; }
inline uint8_t imm8(uint16_t inst) { return inst; }
inline uint16_t imm12(uint16_t inst) { return inst&0xFFF; }


inline void Chip8::cls() {
    mBoy.clear();
}

inline void Chip8::ret() {
    if(mSP == 0) {
        Serial.print(F("STACK UNDERFLOW -- "));
        Serial.println(mPC, HEX);
        halt();
        return;
    }
    mSP--;
    mPC = mStack[mSP];
}

inline void Chip8::halt() {
    mRunning = false;
}

inline void Chip8::setHires(bool enabled) {
    mHires = enabled;
}

inline void Chip8::groupSys(uint16_t inst) {
    switch(inst) {
        case 0x00E0: return cls();
        case 0x00EE: return ret();
        case 0xFB: return scrollRight();
        case 0xFC: return scrollLeft();
        case 0xFD: return halt();
        case 0x00FE: return setHires(false);
        case 0x00FF: return setHires(true);
        default:
            unimpl(inst);
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
        Serial.print(F("STACK OVERFLOW -- "));
        Serial.println(mPC, HEX);
        mRunning = false;
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
}

inline void Chip8::unimpl(uint16_t inst) {
    Serial.println(inst, HEX);
    mRunning = false;
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

inline void Chip8::aluShr(uint8_t x) { 
    mV[0xF] = mV[x]&0x1 ? 1 : 0;
    mV[x] >>= 1;
}

inline void Chip8::aluShl(uint8_t x) { 
    mV[0xF] = mV[x]&0x80 ? 1 : 0;
    mV[x] <<= 1;
}



void Chip8::groupALU(uint16_t inst) {
    switch (inst&0xF) {
        case 0x0: return aluLd(x(inst), y(inst));
        case 0x1: return aluOr(x(inst), y(inst));
        case 0x2: return aluAnd(x(inst), y(inst));
        case 0x3: return aluXor(x(inst), y(inst));
        case 0x4: return aluAdd(x(inst), y(inst));
        case 0x5: return aluSub(x(inst), y(inst));
        case 0x6: return aluShr(x(inst));
        case 0x7: return aluSubn(x(inst), y(inst));
        case 0xE: return aluShl(x(inst));
        default:
            unimpl(inst);
    }
}

// 0x9xxx - Skip if two xisters hold inequal values
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
    unimpl(inst);
    //mPC = mI+imm12(inst);
}

//0xCxxx random
void Chip8::groupRand(uint16_t inst) {
    mV[x(inst)] = random(1+imm8(inst));
}

//0xDxxx - draw
void Chip8::groupGraphics(uint16_t inst) {
    uint8_t rows = imm4(inst);
    uint8_t xc = mV[x(inst)];
    uint8_t yc = mV[y(inst)];
    uint8_t cols = 8;
    uint16_t mask = 0x80;
    if(rows == 0 && mHires) {
       rows = 16;
       mask = 0x8000;
       cols = 16;
    }

    bool collide = false;

    uint8_t scale = mHires ? 1 : 2;
    mV[0xF] = 0;
    for(int row = 0; row < rows; row++) {
        uint16_t rowData;
        if(mHires && rows == 16) {
            rowData = readMem(mI+row*2);
            rowData <<= 8;
            rowData |= readMem(mI+row*2+1);
        } else {
            rowData = readMem(mI+row);
        }
        for(int col = 0; col < cols; col++) {
            bool on = rowData&mask;
            uint8_t py = scale*((yc+row)%(64/scale));
            uint8_t px = scale*((xc+col)%(128*scale));

            bool wasOn = (mBoy.getPixel(px, py) == WHITE);
            uint8_t newColor = wasOn ^ on ? WHITE : BLACK;
            mV[0xF] |= (wasOn & on) ? 1 : 0;
            mBoy.drawPixel(px,py, newColor);
            if(!mHires) {
                mBoy.drawPixel(px+1,py, newColor);
                mBoy.drawPixel(px,py+1, newColor);
                mBoy.drawPixel(px+1,py+1, newColor);
            }
            // 
            rowData<<=1;

        }
    }
    mBoy.display();
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
            unimpl(inst);
    }
}

uint8_t Chip8::readMem(uint16_t addr) {

    if(mHires && addr < sizeof(fonthi)) {
        return pgm_read_byte(&fonthi[addr]);
    }

    if(addr < sizeof(font)) {
        return pgm_read_byte(&font[addr]);
    }

    uint8_t val;
    if(readCell(addr, &val)) {
        writeCell(addr, val);
        return val;
    }

    if(addr < mProgramSize) {
        return pgm_read_byte(&mProgram[addr-0x200]);
    } else {
        if(addr-mProgramSize > MEM_SIZE) {
            Serial.print("OVER READ ");
            Serial.print(addr, HEX);
            mRunning = false;
        }
        return mM[addr-mProgramSize];
    }
}

bool Chip8::readCell(uint16_t addr, uint8_t *val) {
    for(int i = 0; i < mCellIndex; i++) {
        if(mCellAddrs[i] == addr) {
            *val = mCellValues[i];
            return true;
        }
    }
    return false;
}

void Chip8::writeCell(uint16_t addr, uint8_t val) {
    for(int i = 0; i < mCellIndex; i++) {
        if(mCellAddrs[i] == addr) {
            mCellValues[i] = val;
            return; 
        }
    }
    if(mCellIndex >= MAX_CELLS) {
        Serial.println(F("NO MORE CELLS"));
        return;
    }
    Serial.print("ADD CELL ");
    Serial.println(addr, HEX);
    mCellAddrs[mCellIndex] = addr;
    mCellValues[mCellIndex++] = val;
}

void Chip8::writeMem(uint16_t addr, uint8_t val) {
    if(addr < mProgramSize) {
        writeCell(addr, val);
    } else {
        if(addr-mProgramSize > MEM_SIZE) {
            Serial.print("OVERWRITE ");
            Serial.println(addr, HEX);
            mRunning = false;
        }
        mM[addr-mProgramSize] = val;
    }
}

inline void Chip8::readDT(uint8_t into) { mV[into] = mDT; }
inline void Chip8::waitK(uint8_t into) {
    mRunning = false;
    mWaitKey = true;
}
inline void Chip8::setDT(uint8_t from) { mDT = mV[from]; }
inline void Chip8::makeBeep(uint16_t dur) { beep.tone(beep.freq(880), dur); }
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
        default:
            unimpl(inst);
    }
}

inline void Chip8::scrollLeft() {
    // Screen buffer is laid out in 8 pages,
    // page 0 is rows 0-7, all columns, imagine a sequence of vertical bytes
    // similar for page 1-7; 
    for(int page = 0; page < 8; page++) {
        for(int col=0; col < WIDTH-4; col++) {
            mBoy.sBuffer[page*WIDTH + col] = mBoy.sBuffer[page*WIDTH + col + 4];
        }
        for(int col=WIDTH-4; col < WIDTH; col++) {
            mBoy.sBuffer[page*WIDTH + col] = 0;
        }
    }
    mBoy.display();
}

inline void Chip8::scrollRight() {
    unimpl(mPC);
}

