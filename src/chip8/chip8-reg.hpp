#pragma once

#include <stdint.h>

// A set of helpers for reading data from instruction words.

// Returns the X register, which is always the 2nd nybble.
inline uint8_t x(uint16_t inst) { return (inst>>8)&0x0F; }

// Returns the Y register, which is always the 3nd nybble.
inline uint8_t y(uint16_t inst) { return (inst>>4)&0x0F; }

// Returns 4-bit immediate value (last nybble).
inline uint8_t imm4(uint16_t inst) { return inst&0x0F; }

// Returns 8-bit immediate value (last byte).
inline uint8_t imm8(uint16_t inst) { return inst; }

// Returns 12-bit immediate value (last 3 nybbles).
inline uint16_t imm12(uint16_t inst) { return inst&0xFFF; }
