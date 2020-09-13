#pragma once

#include "../src/chip8/render.hpp"
#include "SDL2/SDL.h"

class SDLRender : public Render {
    // Button map 0-F little-endian
    uint16_t mButtons = 0;

    SDL_Renderer *mRenderer;

    uint8_t mScrollX = 0;
    uint8_t mScrollY = 0;

    uint8_t mWidth = 64;
    uint8_t mHeight = 32;

    // Store pixel data ourselves, so we can do XORing, and maybe other cool things like
    // phosphor decay. It's not trivial to read the pixel data back from the SDL_Texture.
    uint32_t mPixelData[64][128];

    public:
    SDLRender(SDL_Renderer* renderer);
    ~SDLRender();
    virtual void render();

    virtual void setMode(RenderMode mode);

    virtual bool drawPixel(uint8_t x, uint8_t y, bool drawVal);
    virtual void clear();

    // 128 x 64 texture. Clipped for Chip8 mode.
    SDL_Texture * mTexture;

    void setKeyState(uint8_t button, bool pressed) { 
        uint16_t mask = 1 << button;
        mButtons = pressed ? mButtons | mask : mButtons & ~mask;
    }

    // SUPER CHIP-8
    virtual void scrollDown(uint8_t amt);
    virtual void scrollLeft();
    virtual void scrollRight();

    // Non-drawing rendering
    virtual void beep(uint8_t dur);
    virtual uint8_t random();
    virtual uint16_t buttons();
};
