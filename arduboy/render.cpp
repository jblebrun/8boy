#include "render.hpp"

ArduboyRender::ArduboyRender(Arduboy2 &boy) : mBoy(boy) {
    beepPin.begin();
}

// Progress the beep timer on ticks.
void ArduboyRender::tick() {
    beepPin.timer();
}

void ArduboyRender::setMode(RenderMode mode) {
    mMode = mode; 
    switch(mMode) {
        case CHIP8:
            mPixelWidth = 2;
            mPixelHeight = 2;
            break;
        case CHIP8HI:
            mPixelWidth = 2;
            mPixelHeight = 1;
            break;
        case SCHIP8:
            mPixelWidth = 1;
            mPixelHeight = 1;
            break;
    }
}

// Draw the pixel specified by Chip8 coordinates cx, cy with chip8 draw value
// drawVal.
bool ArduboyRender::drawPixel(uint8_t cx, uint8_t cy, bool drawVal) {
    // Get the top left coordinates of the pixel rectangle we need to draw.
    uint8_t x = (mPixelWidth * cx) % 128;
    uint8_t y = (mPixelHeight * cy) % 64;

    // Get the current pixel value, so we can xor it.
    bool wasOn = mBoy.getPixel(x, y) == WHITE;

    // Calculate the new pixel value.
    uint8_t on = wasOn ^ drawVal;

    // Draw the pixel rectangle.
    // Use a series of drawPixel calls, seems fastest.
    mBoy.drawPixel(x, y, on ? WHITE : BLACK);
    if(mPixelWidth > 1) {
        mBoy.drawPixel(x+1, y, on ? WHITE : BLACK);
        if(mPixelHeight > 1) mBoy.drawPixel(x+1, y+1, on ? WHITE : BLACK);
    }
    if(mPixelHeight > 1) mBoy.drawPixel(x, y+1, on ? WHITE : BLACK);

    // A pixel draw triggers a collision when it moves from set to unset.
    return wasOn & !on;
}

// Screen buffer is laid out in 8 pages,
// page 0 is rows 0-7, all columns, imagine a sequence of vertical bytes
// similar for page 1-7; 
// So for example (using just 4 bits instead of 8), for the first page, if you
// screen has pixel values:
//
// ABCDEFGH
// IJKLMNOP
// QRSTUVWX
// YZ123456
//
// The memory layout is:
// [AIQY, BJRZ, CKS1, DLT2, EMU3, FNV4, GOW5, HPX6]
//
// WIDTH is an Ardubboy2-provided constant.

    
// Implement the scrollDown function. The underlying hardware doesn't support
// it, so we need to actually copy the memory column by column.
inline void ArduboyRender::scrollDown(uint8_t shift) {
    
    // We work from the bottom up, shifting the bits of the page, and then
    // pulling in the bits that are about to be shifted from the page above it.
    
    // The number of pages to skip when moving the pixels down.
    uint8_t skip = shift / 8;

    // The number of bits to shift an individual column.
    shift = shift % 8;

    // Working from the bottom up
    for(int page = 7; page >= 0; page--) {
       for(int col = 0; col < WIDTH; col++) {
           // First, shift the bits of the page we're working with down
           mBoy.sBuffer[page*WIDTH+col] <<= shift;
           // Now , get the bits that are about to be shifted out from the page above.
           uint8_t above = (page > skip) ? mBoy.sBuffer[(page-1-skip)*WIDTH+col] >> (8-shift) : 0;
           // Or those bits with this page.
           mBoy.sBuffer[page*WIDTH+col] |= above;
       } 
    }
}

// Implement the chip8 scrollLeft function. This is easier than up/down, since
// we just have to move the columns. The shift is always by 4.
inline void ArduboyRender::scrollLeft() {
    // See above for mem layout notes.
    for(int page = 0; page < 8; page++) {
        uint8_t* start = mBoy.sBuffer + page * WIDTH;
        // Move the columns that are staying on screen to the left.
        memmove(start, start+4,  WIDTH-4);
        // Clear the columsn that moved offscreen.
        memset(start + WIDTH-4, 0, 4);
    }
}

// Implement the chip8 scrolRight function. This is easier than up/down, since
// we just have to move the columns. The shift is always by 4.
inline void ArduboyRender::scrollRight() {
    // See above for mem layout notes.
    for(int page = 0; page < 8; page++) {
        uint8_t* start = mBoy.sBuffer + page * WIDTH;
        // Move the columns that are staying on screen to the left.
        memmove(start + 4, start,  WIDTH-4);
        // Clear the columsn that moved offscreen.
        memset(start, 0, 4);
    }
}

// Translate the Chip8 beep duration into an Arduboy beep.
void ArduboyRender::beep(uint8_t dur) {
    if(dur == 0) {
        beepPin.noTone();
    } else {
        beepPin.tone(beepPin.freq(800), dur);
    }
}

// Redraw the display.
void ArduboyRender::render() {
    mBoy.display();
}

// Clear all pixels on the display.
void ArduboyRender::clear() {
    mBoy.clear();
}

// Random implementation.
uint8_t ArduboyRender::random() {
    return ::random(0xFF);
}

// Set the keymapping that will be used by the buttons method.
void ArduboyRender::setKeyMap(uint8_t ud, uint8_t lr, uint8_t ab) {
    mKeymapUD = ud;
    mKeymapLR = lr;
    mKeymapAB = ab;
}

// Return the button state.
uint16_t ArduboyRender::buttons() {
    mBoy.pollButtons();
    uint16_t buttons = 0;
    if(mBoy.pressed(UP_BUTTON)) buttons |= (1 << (mKeymapUD >> 4));
    if(mBoy.pressed(DOWN_BUTTON)) buttons |= (1 << (mKeymapUD & 0xF));
    if(mBoy.pressed(LEFT_BUTTON)) buttons |= (1 << (mKeymapLR >> 4));
    if(mBoy.pressed(RIGHT_BUTTON)) buttons |= (1 << (mKeymapLR & 0xF));
    if(mBoy.pressed(A_BUTTON)) buttons |= (1 << (mKeymapAB >> 4));
    if(mBoy.pressed(B_BUTTON)) buttons |= (1 << (mKeymapAB & 0xF));
    return buttons;
}
