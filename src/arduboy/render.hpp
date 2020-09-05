#pragma once

#include <Arduboy2.h>
#include "../chip8/render.hpp"


class ArduboyRender : public Render {

private:
    Arduboy2 &mBoy;
    BeepPin1 beepPin;
    
    void message(uint16_t addr, uint16_t inst);

    uint8_t mPixelWidth = 2; 
    uint8_t mPixelHeight = 2;

public:
    ArduboyRender(Arduboy2 &boy);
    void tick();
    virtual void setMode(RenderMode mode);
    virtual bool drawPixel(uint8_t x, uint8_t y, bool drawVal);
    virtual void scrollDown(uint8_t amt);
    virtual void scrollLeft();
    virtual void scrollRight();

    virtual void beep(uint8_t dur);
    virtual void render();
    virtual void clear();
    virtual void exit();
    virtual uint8_t random();
};
