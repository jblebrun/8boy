#include "chip8.hpp"
#include <Arduino.h>


Chip8::Chip8(Arduboy2 &boy) : mBoy(boy) {}

void Chip8::Load(const uint8_t *program) {
    Serial.println("START PROGRAM");
    mProgram = program;
}

void Chip8::Reset() {
    mWrites = 0;
    mPC = 0x200;
    mSP = 0;
    mRunning = true;
}

bool Chip8::Running() {
    return mRunning;
}

void Chip8::Toggle() {
    mRunning = !mRunning;
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

    mPC+=2;
    /*
    Serial.print("INST ");
    Serial.print(mPC, HEX);
    Serial.print(" - ");
    Serial.println(inst, HEX);
    */
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

// XXX - system instructions
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
            Serial.print(mPC, HEX);
            Serial.print(" RET ");
            mPC = mStack[mSP];
            Serial.println(mPC, HEX);

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
    Serial.print(mPC, HEX);
    Serial.print(" CALL ");
    mPC = imm12(inst);
    Serial.println(mPC, HEX);
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
    mV[reg(inst)] += (uint8_t)inst;
}

void Chip8::unimpl(uint16_t inst) {
    Serial.print("UN ");
    Serial.println(inst, HEX);
    mRunning = false;
}

//0x8xx0 - ALU
void Chip8::groupALU(uint16_t inst) {
    // XXX lots to do here
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
            mV[reg(inst)] = mV[reg(inst)] + mV[reg2(inst)];
            break;
        case 0x5: // Vx = Vx - Vy
            mV[reg(inst)] = mV[reg2(inst)] - mV[reg(inst)];
            // XXX - !borrow
            mV[0xF] = mV[reg(inst)] > mV[reg2(inst)] ? 0 : 1;
            break;
        case 0x6: // Vx = Vx SHR 1
            mV[reg(inst)] >>= 1;
            break;
        case 0x7:
            mV[reg(inst)] = mV[reg2(inst)] - mV[reg(inst)];
            // XXX - !borrow
            mV[0xF] = mV[reg(inst)] > mV[reg2(inst)] ? 0 : 1;
            break;
        case 0xE: // Vx = Vx SHL 1
            Serial.print("SHL ");
            Serial.print(mV[reg(inst)], HEX);
            mV[reg(inst)] <<= 1;
            Serial.print(" -> ");
            Serial.println(mV[reg(inst)], HEX);
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
    Serial.print("SET I - ");
    Serial.println(mI, HEX);
}

// 0xBxxx jump to I + xxx
void Chip8::groupJpV0Index(uint16_t inst) {
    mPC = mI+imm12(inst);
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
    uint16_t spriteStart = (mI-0x200);
    Serial.print("DRAW ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(" - ");
    Serial.print(rows);
    Serial.print(" rows at ");
    Serial.println(mI, HEX);

    for(int row = 0; row < rows; row++) {
        uint8_t rowData = pgm_read_byte(&mProgram[spriteStart+row]);
        Serial.print("ROW");
        Serial.print(row);
        Serial.print(" at ");
        Serial.print(spriteStart+row, HEX);
        Serial.print(" - ");
        Serial.println(rowData, HEX);
        for(int col = 0; col < 8; col++) {
            bool on = rowData&0x80;
            /*
            Serial.print("PIXEL ");
            Serial.print(x+col);
            Serial.print(", ");
            Serial.print(y+row);
            Serial.print(" ");
            Serial.print(rowData, HEX);
            Serial.print(" -- ");
            Serial.println(on);
            */
            mBoy.fillRect(2*(x+col), 2*(y+row), 2,2, on ? WHITE : BLACK);
            rowData<<=1;
        }
    }
    mBoy.display();
}

// 0xExxx XXX -keyboard to keys
void Chip8::groupKeyboard(uint16_t inst) {
    uint8_t key = mV[reg(inst)];
    switch(imm8(inst)) {
        case 0x9E:
            break;
        case 0xA1:
            mPC+=2;
            break;
        default:
            unimpl(inst);
    }
}

// 0xFxxx XXX - loady things
void Chip8::groupLoad(uint16_t inst) {
    uint8_t to = reg(inst);
    switch(inst&0xFF) {
        case 0x07:
            mV[to] = mDT;
            break;
        case 0x1E:
            Serial.print("ADD TO I: ");
            Serial.println(mV[to], HEX);
            mI = mI + mV[to];
            break;
        // Store registers to memory
        case 0x55:
            for(int i = 0; i <= to; i++) {
                Serial.print("MEMORY WRITE ");
                Serial.print(mWrites++);
                Serial.print(" ");
                Serial.print(mI+i, HEX);
                Serial.print(" = ");
                Serial.println(mV[i], HEX);
                if(mI+i < 0xB34) {
                    Serial.println("Can't write");
                } else {
                    mM[mI+i-0xB34] = mV[i];
                }
            }
            break;
        // Read registers from memory
        case 0x65:
            for(int i = 0; i <= to; i++) {
                Serial.print("MEMORY READ "); 
                Serial.print(i);
                Serial.print(" FROM ");
                if(mI + i < 0xB34) {
                    Serial.print("FLASH - ");
                    mV[i] = pgm_read_byte(&mProgram[mI+i-0x200]);
                } else {
                    Serial.print("RAM - ");
                    mV[i] = mM[mI+i-0xB34];
                }
                Serial.print(mI+i, HEX);
                Serial.print(" = ");
                Serial.println(mV[i], HEX);
            }
            break;
        default:
            unimpl(inst);
    }
}

