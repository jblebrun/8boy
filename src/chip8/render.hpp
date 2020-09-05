#pragma once
#include <stdint.h>

enum RenderMode { CHIP8, CHIP8HI, SCHIP8 };

// These methods need to be implemented to provide the drawing, sound, and
// random functionality that the Chip8 engine needs.
//
// Also includes a hook for any special exit behavior.
class Render {
    protected: 
    RenderMode mMode;

    public:
    // if it should trigger a collision, return true.
    virtual bool drawPixel(uint8_t x, uint8_t y, bool drawVal) = 0;

    // set the resolution mode
    virtual void setMode(RenderMode mode) { mMode = mode; }

    // get the resolution mode
    virtual RenderMode mode() { return mMode; }

    // draw the screen
    virtual void render() = 0;

    // clear the screen
    virtual void clear() = 0;
    

    // SUPER CHIP-8
    
    // scroll the display down the specified number of lines
    virtual void scrollDown(uint8_t amt) = 0;
    
    // scroll the display left 4 columns
    virtual void scrollLeft() = 0;
    
    // scroll the display right 4 columns
    virtual void scrollRight() = 0;

    // implement if you want to perform custom actions when emulator exits
    virtual void exit() = 0;


    // Non-drawing rendering
    
    // implementations should make a noise for dur/60 seconds.
    virtual void beep(uint8_t dur) = 0;

    // implementations should return a uniform random number from 0 - 0xFF
    virtual uint8_t random() = 0;

    // return the button state. Should return a bitmask for the buttons that have 
    // been pressed, in little-endian order for 0-F.
    virtual uint16_t buttons() = 0;
};    
