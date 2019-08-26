#pragma once
#include <stdint.h>

class Render {
    public:
    // implementations should return the current value of the pixel at the given coordinate.
    virtual bool getPixel(uint8_t x, uint8_t y) = 0;
    // implementations should change the state of the pixel at the given coordinate 
    virtual void drawPixel(uint8_t x, uint8_t y, bool on) = 0;
    // implementatoins should make a noise for dur/60 seconds.
    virtual void beep(uint8_t dur) = 0;
    // implementations should draw the screen
    virtual void render() = 0;
    // implementations should clear the screen
    virtual void clear() = 0;
    // implementations should return a uniform random number from 0 - 0xFF
    virtual uint8_t random() = 0;

    // SUPER CHIP-8
    // implementations should scroll the display down the specified number of lines
    virtual void scrollDown(uint8_t amt) = 0;
    // implementations should scroll the display left 4 columns
    virtual void scrollLeft() = 0;
    // implementations should scroll the display right 4 columns
    virtual void scrollRight() = 0;

    // implement if you want to perform custom actions when emulator exits
    virtual void exit() = 0;

    // errors
    // override this implementations if you want to perform custom
    // behavior when the emulator encounters an error
    // emulator will be in stopped state 
    virtual void stackUnderflow(uint16_t addr) {};
    virtual void stackOverflow(uint16_t addr) {};
    virtual void oom(uint16_t addr) {};
    virtual void badread(uint16_t addr) {};
    virtual void unimpl(uint16_t addr, uint16_t inst) {};
};
