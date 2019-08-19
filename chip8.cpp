#include "chip8.hpp"
#include <Arduino.h>


Chip8::Chip8(Arduboy2 &boy) : mBoy(boy) {}

void Chip8::Load(uint16_t *program) {
    Serial.println("START PROGRAM");
    mProgram = (uint8_t*)program;
}

void Chip8::Reset() {
    mPC = 0x200;
}

void Chip8::Step() {
    // endian fix
    uint16_t inst = (uint16_t(mProgram[mPC+1-0x200]) << 8) | (mProgram[mPC-0x200]) ;
    mPC+=2;
    Serial.print("INST ");
    Serial.print(mPC, HEX);
    Serial.print(" - ");
    Serial.println(inst, HEX);
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
void Chip8::groupSys(uint16_t) {
}

// 0x1xxx jump
void Chip8::groupJump(uint16_t inst) {
    mPC = imm12(inst);
}

// 02xxx call
void Chip8::groupCall(uint16_t inst) {
    // XXX - stack adjust
    mPC = imm12(inst);
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

//0x8xx0 - ALU
void Chip8::groupALU(uint16_t inst) {
    // XXX lots to do here
    switch (inst&0xF) {
        case 0: // LD Vx, Vy
            mV[reg(inst)] = mV[reg2(inst)];
            break;
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
    uint8_t spriteStart = (mI-0x200);
    Serial.print("DRAW ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(" - ");
    Serial.print(rows);
    Serial.print(" rows at ");
    Serial.println(spriteStart);

    for(int row = 0; row < rows; row++) {
        uint8_t rowData = mProgram[spriteStart+row];
        /*
        Serial.print("ROW");
        Serial.print(row);
        Serial.print(" - ");
        Serial.println(rowData, HEX);
        */
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
void Chip8::groupKeyboard(uint16_t) {
}

// 0xFxxx XXX - loady things
void Chip8::groupLoad(uint16_t) {
}

