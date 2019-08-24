#include "chip8.hpp"
#include <Arduino.h>


const uint16_t FONT_OFFSET = 0x10 * 5;
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

Chip8::Chip8(Arduboy2 &boy) : mBoy(boy) {
    beep.begin();
}

void Chip8::Load(const uint8_t *program, const uint16_t size) {
    Serial.println("START PROGRAM");
    mProgram = program;
    mProgramSize = size;
}

void Chip8::Reset() {
    Serial.println("Reset");
    mWrites = 0;
    mPC = 0x200;
    mSP = 0;
    mRunning = true;
    mBoy.clear();
}

bool Chip8::Running() {
    return mRunning;
}

void Chip8::Toggle() {
    mRunning = !mRunning;
}

void Chip8::Buttons(uint8_t buttons) {
    beep.timer();
    if(mDT > 0) {
        mDT--;
    }
    
    mButtons = buttons;
    if (mWaitKey && buttons) {
        mWaitKey = false;
        mRunning = true;
    }
}

void Chip8::Step() {
    // endian fix
    uint8_t hi = pgm_read_byte(&mProgram[mPC-0x200]);
    uint8_t lo = pgm_read_byte(&mProgram[mPC+1-0x200]);
    uint16_t inst = (uint16_t(hi << 8) | lo) ;

    if(inst == lastinst) {
        mRunning = false;
        return;
    }

    /*
    Serial.print("INST ");
    Serial.print(mPC, HEX);
    Serial.print(" - ");
    Serial.println(inst, HEX);
    */
    mPC+=2;
    uint8_t group = uint8_t(inst >> 12); 
    groupFunc gf = groupFuncs[group];
    (this->*gf)(inst);
}

inline uint8_t reg(uint16_t inst) {
    return (inst>>8)&0x0F;
}

inline uint8_t reg2(uint16_t inst) {
    return (inst>>4)&0x0F;
}

inline uint8_t imm4(uint16_t inst) {
    return inst&0x0F;
}

inline uint8_t imm8(uint16_t inst) {
    return inst;
}

inline uint16_t imm12(uint16_t inst) {
    return inst&0xFFF;
}

void Chip8::groupSys(uint16_t inst) {
    switch(inst) {
        // CLS
        case 0x00E0:
            mBoy.clear();
            break;
        // RET
        case 0x00EE:
            // What does original interpreter do?
            if(mSP == 0) {
                Serial.print("STACK UNDERFLOW -- ");
                Serial.println(mPC, HEX);
                mRunning = false;
                return;
            }
            mSP--;
            //Serial.print(mPC, HEX);
            //Serial.print(" RET ");
            mPC = mStack[mSP];
            //Serial.println(mPC, HEX);

            break;
        default:
            unimpl(inst);
    }
}

// 0x1xxx jump
void Chip8::groupJump(uint16_t inst) {
    mPC = imm12(inst);
}

// 02xxx call
void Chip8::groupCall(uint16_t inst) {
    // What does original interpreter do?
    if(mSP > 16) {
        Serial.print("STACK OVERFLOW -- ");
        Serial.println(mPC, HEX);
        mRunning = false;
        return;
    }
    mStack[mSP++] = mPC;
    //Serial.print(mPC, HEX);
    //Serial.print(" CALL ");
    mPC = imm12(inst);
    //Serial.println(mPC, HEX);
}

// 0x3000 skip if equal immediate
void Chip8::groupSeImm(uint16_t inst) {
    if(mV[reg(inst)] == imm8(inst)) {
        mPC+=2;
    }
}

// 0x4xxxx skip if not equal immediate
void Chip8::groupSneImm(uint16_t inst) {
    if(mV[reg(inst)] != imm8(inst)) {
        mPC+=2;
    }
}

// 0x5xxx skip if two registers hold equal values
void Chip8::groupSeReg(uint16_t inst) {
    if(mV[reg(inst)] == mV[reg2(inst)]) {
        mPC+=2;
    }
}

//0x6xxx
void Chip8::groupLdImm(uint16_t inst) {
    mV[reg(inst)] = (uint8_t)inst;
}

//0x7xxx add immediate
void Chip8::groupAddImm(uint16_t inst) {
    mV[reg(inst)] += imm8(inst);
}

void Chip8::unimpl(uint16_t inst) {
    Serial.print("UN ");
    Serial.println(inst, HEX);
    mRunning = false;
}

//0x8xx0 - ALU
void Chip8::groupALU(uint16_t inst) {
    uint16_t result_carry;
    switch (inst&0xF) {
        case 0x0: // LD Vx, Vy
            mV[reg(inst)] = mV[reg2(inst)];
            break;
        case 0x1: // Vx = Vx | Vy
            mV[reg(inst)] = mV[reg(inst)] | mV[reg2(inst)];
            break;
        case 0x2: // Vx = Vx & Vy
            mV[reg(inst)] = mV[reg(inst)] & mV[reg2(inst)];
            break;
        case 0x3: // Vx = Vx ^ Vy
            mV[reg(inst)] = mV[reg(inst)] ^ mV[reg2(inst)];
            break;
        case 0x4: // Vx = Vx + Vy
            result_carry = mV[reg(inst)] + mV[reg2(inst)];
            mV[0xF] = result_carry > 0x00FF ? 1 : 0;
            mV[reg(inst)] = result_carry;
            break;
        case 0x5: // Vx = Vx - Vy
            mV[0xF] = mV[reg(inst)] > mV[reg2(inst)]; 
            mV[reg(inst)] = mV[reg(inst)] - mV[reg2(inst)];
            break;
        case 0x6: // Vx = Vx SHR 1
            mV[0xF] = mV[reg(inst)]&0x1 ? 1 : 0;
            mV[reg(inst)] >>= 1;
            break;
        case 0x7: // Vx = Vy - Vx
            mV[0xF] = mV[reg2(inst)] > mV[reg(inst)]; 
            mV[reg(inst)] = mV[reg2(inst)] - mV[reg(inst)];
            break;
        case 0xE: // Vx = Vx SHL 1
            mV[0xF] = mV[reg(inst)]&0x8 ? 1 : 0;
            mV[reg(inst)] <<= 1;
            break;
        default:
            unimpl(inst);
    }
}

// 0x9xxx - Skip if two registers hold inequal values
void Chip8::groupSneReg(uint16_t inst) {
    if(mV[reg(inst)] != mV[reg2(inst)]) {
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
    mV[reg(inst)] = random(1+imm8(inst));
}

//0xDxxx - draw
void Chip8::groupGraphics(uint16_t inst) {
    uint8_t rows = imm4(inst);
    uint8_t x = mV[reg(inst)];
    uint8_t y = mV[reg2(inst)];

    bool collide = false;

    mV[0xF] = 0;
    for(int row = 0; row < rows; row++) {
        uint8_t rowData = readMem(mI+row);
        for(int col = 0; col < 8; col++) {
            bool on = rowData&0x80;
            uint8_t py = 2*((y+row)%32);
            uint8_t px = 2*((x+col)%64);

            bool wasOn = (mBoy.getPixel(px, py) == WHITE);
            uint8_t newColor = wasOn ^ on ? WHITE : BLACK;
            mV[0xF] |= (wasOn & on) ? 1 : 0;
            mBoy.drawPixel(px,py, newColor);
            mBoy.drawPixel(px+1,py, newColor);
            mBoy.drawPixel(px,py+1, newColor);
            mBoy.drawPixel(px+1,py+1, newColor);
            // 
            rowData<<=1;

        }
    }
    mBoy.display();
}

// 0xExxx XXX -keyboard to keys
void Chip8::groupKeyboard(uint16_t inst) {
    uint8_t key = mV[reg(inst)];
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
    uint8_t val;
    if(readCell(addr, &val)) {
        writeCell(addr, val);
        return val;
    }
    if(addr < FONT_OFFSET) {
        return pgm_read_byte(&font[addr]);
    } else if(addr < mProgramSize) {
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

void Chip8::groupLoad(uint16_t inst) {
    uint8_t to = reg(inst);
    switch(inst&0xFF) {
        case 0x07:
            mV[to] = mDT;
            break;
        case 0xA:
            mRunning = false;
            mWaitKey = true;
            break;
        case 0x15:
            mDT = mV[to];
            break;
        case 0x18:
            beep.tone(beep.freq(880), imm8(inst));
            break;

        case 0x1E:
            mI = mI + mV[to];
            break;

        case 0x29:
            
            mI = 5 * mV[to];
            break;
        case 0x33: {
            uint8_t val = mV[to];
            writeMem(mI, val/100);
            writeMem(mI+1, (val/10)%10);
            writeMem(mI+2, val%10);
            break;
        }

        // Store registers to memory
        case 0x55:
            for(int i = 0; i <= to; i++) {
                /*
                Serial.print("MEMORY WRITE ");
                Serial.print(mWrites++);
                Serial.print(" ");
                Serial.print(mI+i, HEX);
                Serial.print(" = ");
                Serial.println(mV[i], HEX);
                */
                writeMem(mI+i, mV[i]);
            }
            break;
        // Read registers from memory
        case 0x65:
            for(int i = 0; i <= to; i++) {
                mV[i] = readMem(mI+i);
            }
            break;
        default:
            unimpl(inst);
    }
}

