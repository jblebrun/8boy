#pragma once
struct Program {
    char *name;
    uint8_t *code;
    uint16_t size;
    bool super;
    uint8_t *info;
    uint8_t keymap[3];
    bool shiftquirk;
};
