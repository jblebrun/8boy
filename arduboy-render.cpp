#include "arduboy-render.hpp"

ArduboyRender::ArduboyRender(Arduboy2 &boy) : mBoy(boy) {
    beepPin.begin();
}

void ArduboyRender::tick() {
    beepPin.timer();
}

bool ArduboyRender::getPixel(uint8_t x, uint8_t y) {
    return mBoy.getPixel(x, y) == WHITE;
}

void ArduboyRender::drawPixel(uint8_t x, uint8_t y, bool on) {
    mBoy.drawPixel(x, y, on ? WHITE : BLACK);
}

inline void ArduboyRender::scrollDown(uint8_t amt) {
    uint8_t skip = amt/8;
    for(int page = 7; page >= 0; page--) {
       for(int col = 0; col < WIDTH; col++) {
           mBoy.sBuffer[page*WIDTH+col] <<= amt;
           uint8_t above = (page > skip) ? mBoy.sBuffer[(page-1-skip)*WIDTH+col] >> (8-amt) : 0;
           mBoy.sBuffer[page*WIDTH+col] |= above;
       } 
    }
}

inline void ArduboyRender::scrollLeft() {
    // Screen buffer is laid out in 8 pages,
    // page 0 is rows 0-7, all columns, imagine a sequence of vertical bytes
    // similar for page 1-7; 
    for(int page = 0; page < 8; page++) {
        for(int col=0; col < WIDTH-4; col++) {
            mBoy.sBuffer[page*WIDTH + col] = mBoy.sBuffer[page*WIDTH + col + 4];
        }
        for(int col=WIDTH-4; col < WIDTH; col++) {
            mBoy.sBuffer[page*WIDTH + col] = 0;
        }
    }
}

inline void ArduboyRender::scrollRight() {
}

void ArduboyRender::beep(uint8_t dur) {
    if(dur == 0) {
        beepPin.noTone();
    } else {
        beepPin.tone(beepPin.freq(1200), dur/6);
    }
}

void ArduboyRender::render() {
    mBoy.display();
}

void ArduboyRender::clear() {
    mBoy.clear();
}

void ArduboyRender::stackUnderflow(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("UNDERFLOW "));
    mBoy.print(addr, HEX);
}
void ArduboyRender::stackOverflow(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("OVERFLOW "));
    mBoy.print(addr, HEX);
}
void ArduboyRender::oom(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("OOM "));
    mBoy.print(addr, HEX);
}
void ArduboyRender::badread(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("BAD READ "));
    mBoy.print(addr, HEX);
}
void ArduboyRender::exit() {
    mBoy.setCursor(0,0);
    mBoy.print(F("Press key to restart"));
}

void ArduboyRender::unimpl(uint16_t addr, uint16_t inst) {
    mBoy.setCursor(0,0);
    mBoy.print(F("UNIMPL "));
    mBoy.print(addr, HEX);
    mBoy.print(F(":"));
    mBoy.print(inst, HEX);
}
 
uint8_t ArduboyRender::random() {
    return ::random(0xFF);
}
