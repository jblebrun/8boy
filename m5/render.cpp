#include <M5Stack.h>
#include "render.hpp"


M5Render::M5Render(M5Gamepad &gamepad) : mGamepad(gamepad) {
    mPixelData = (uint16_t*)mSprite.createSprite(256, 128);
}

M5Render::~M5Render() {
    mSprite.deleteSprite();
}

bool M5Render::drawPixel(uint8_t x, uint8_t y, bool drawVal) {
    uint8_t topy = y * mPixelHeight;
    uint8_t leftx = x * mPixelWidth;
    uint16_t offset = (topy * mWidth) + (leftx);
    uint16_t oldPixel = mSprite.readPixel(leftx, topy);
    bool wasOn = oldPixel == 0xFFFF;
    bool on = wasOn ^ drawVal;
    mSprite.fillRect(leftx, topy, mPixelWidth, mPixelHeight, on ? WHITE : 0);
    return wasOn & !on;
}

void M5Render::render() {
    M5.update();
    mSprite.pushSprite(32, 32);
}

void M5Render::setMode(RenderMode mode) {
    Render::setMode(mode);
    mPixelWidth = mode == SCHIP8 ? 2 : 4;
    mPixelHeight = mode == CHIP8 ? 4 : 2;
    mWidth = mode == SCHIP8  ? 128 : 64;
    mHeight = mode == CHIP8 ? 32 : 64;
}

void M5Render::clear() {
    mSprite.fillSprite(BLACK);
}

void M5Render::scrollDown(uint8_t amt) {
    mSprite.scroll(0, amt * mPixelHeight);
}

void M5Render::scrollLeft() {
    mSprite.scroll(-4 * mPixelWidth, 0);
}

void M5Render::scrollRight() {
    mSprite.scroll(4 * mPixelWidth, 0);
}

void M5Render::beep(uint8_t dur) {
    if(dur == 0) {
        M5.Speaker.mute();
    } else {
        M5.Speaker.tone(880, dur);
    }
}

uint8_t M5Render::random() {
    return ::random(0xFF);
}

// Set the keymapping that will be used by the buttons method.
void M5Render::setKeyMap(uint8_t ud, uint8_t lr, uint8_t ab) {
    mKeymapUD = ud;
    mKeymapLR = lr;
    mKeymapAB = ab;
}

uint16_t M5Render::buttons() {
    uint16_t buttons = 0;
    if(mGamepad.pressed(GAMEPAD_UP)) buttons |= (1 << (mKeymapUD >> 4));
    if(mGamepad.pressed(GAMEPAD_DOWN)) buttons |= (1 << (mKeymapUD & 0xF));
    if(mGamepad.pressed(GAMEPAD_LEFT)) buttons |= (1 << (mKeymapLR >> 4));
    if(mGamepad.pressed(GAMEPAD_RIGHT)) buttons |= (1 << (mKeymapLR & 0xF));
    if(mGamepad.pressed(GAMEPAD_A)) buttons |= (1 << (mKeymapAB >> 4));
    if(mGamepad.pressed(GAMEPAD_B)) buttons |= (1 << (mKeymapAB & 0xF));
    return buttons;
}
