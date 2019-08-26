#pragma once

#include <Arduboy2.h>
#include "src/chip8/render.hpp"


class ArduboyRender : public Render {

private:
    Arduboy2 &mBoy;
    BeepPin1 beepPin;
    
    void message(uint16_t addr, uint16_t inst);

public:
    ArduboyRender(Arduboy2 &boy);
    void tick();
    virtual bool getPixel(uint8_t x, uint8_t y); 
    virtual void drawPixel(uint8_t x, uint8_t y, bool on);
    virtual void scrollDown(uint8_t amt);
    virtual void scrollLeft();
    virtual void scrollRight();

    virtual void beep(uint8_t dur);
    virtual void render();
    virtual void clear();
    virtual void stackUnderflow(uint16_t addr);
    virtual void stackOverflow(uint16_t addr);
    virtual void oom(uint16_t addr);
    virtual void badread(uint16_t addr);
    virtual void exit();
    virtual void unimpl(uint16_t addr, uint16_t inst);
    virtual uint8_t random();
};
