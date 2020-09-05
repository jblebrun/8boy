#pragma once
#include <stdint.h>

// These methods need to be implemented to provide the drawing, sound, and
// random functionality that the Chip8 engine needs.
//
// Also includes a hook for any special exit behavior.
class Render {
    public:
    // return the current state of the screen memory at the given coordinate,
    // true if on, false if off.
    virtual bool getPixel(uint8_t x, uint8_t y) = 0;

    // set the screen memory at location x, y to the provided state.
    virtual void drawPixel(uint8_t x, uint8_t y, bool on) = 0;

    // draw the screen
    virtual void render() = 0;

    // clear the screen
    virtual void clear() = 0;

    // implementations should make a noise for dur/60 seconds.
    virtual void beep(uint8_t dur) = 0;

    // implementations should return a uniform random number from 0 - 0xFF
    virtual uint8_t random() = 0;

    
    // SUPER CHIP-8
    
    // scroll the display down the specified number of lines
    virtual void scrollDown(uint8_t amt) = 0;
    
    // scroll the display left 4 columns
    virtual void scrollLeft() = 0;
    
    // scroll the display right 4 columns
    virtual void scrollRight() = 0;

    // implement if you want to perform custom actions when emulator exits
    virtual void exit() = 0;
};
