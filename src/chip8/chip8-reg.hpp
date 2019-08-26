#pragma once

#include <stdint.h>

// inst read helpers
inline uint8_t x(uint16_t inst) { return (inst>>8)&0x0F; }
inline uint8_t y(uint16_t inst) { return (inst>>4)&0x0F; }
inline uint8_t imm4(uint16_t inst) { return inst&0x0F; }
inline uint8_t imm8(uint16_t inst) { return inst; }
inline uint16_t imm12(uint16_t inst) { return inst&0xFFF; }
