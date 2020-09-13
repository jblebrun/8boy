#include "render.hpp"
#include <cstdlib>
#include <ctime> 
#include <algorithm>
#include "stdio.h"

SDLRender::SDLRender(SDL_Renderer *renderer) :mRenderer(renderer) {
    srand(time(NULL));
    mTexture = SDL_CreateTexture(
            mRenderer,
            SDL_PIXELFORMAT_ARGB8888, 
            SDL_TEXTUREACCESS_STREAMING, 
            128,
            64
    );
    setMode(CHIP8);
}

SDLRender::~SDLRender() {
    SDL_DestroyTexture(mTexture);
}

void SDLRender::setMode(RenderMode mode) {
    Render::setMode(mode);
    mWidth = mode == SCHIP8 ? 128 : 64;
    mHeight = mode == CHIP8 ? 32 : 64;
    SDL_RenderSetLogicalSize(mRenderer, mWidth, mHeight);
}

bool SDLRender::drawPixel(uint8_t x, uint8_t y, bool drawVal) {
    // Don't draw out of bounds.
    if(x < 0 || y < 0 || x >= mWidth || y >= mHeight) return false;

    bool wasOn = mPixelData[y][x] == 0xFFFFFFFF;
    bool on = wasOn ^ drawVal;
    mPixelData[y][x] = on ? 0xFFFFFFFF : 0;

    return wasOn & !on;
}

void SDLRender::clear() {
    SDL_Rect full = {0,0,mWidth, mHeight};
    memset(mPixelData, 0, 128*64*4);
}

void SDLRender::render() {
    // TODO - Handle scroll during texture copy via offsets,
    // so that scrollleft/scrollright don't have to move data.
    SDL_Rect full = {0,0,128,64};
    uint32_t *outPixels;
    int pitch;
    SDL_LockTexture(mTexture, &full, (void**)&outPixels, &pitch);
    memcpy(outPixels, mPixelData, pitch*64);
    SDL_UnlockTexture(mTexture);

    SDL_Rect src = {0, 0, mWidth, mHeight};
    SDL_RenderCopy(mRenderer, mTexture, &src, NULL);
    SDL_RenderPresent(mRenderer);
}

void SDLRender::scrollDown(uint8_t amt) {
    memmove(&mPixelData[amt][0], &mPixelData[0][0], 128*(60-4)*4);
}

void SDLRender::scrollLeft() {
    for(uint8_t y = 0; y < mHeight; y++) {
        memmove(&mPixelData[y][0], &mPixelData[y][4],124 *4);
        memset(&mPixelData[y][mWidth-4], 0, 4*4);
    }
}

void SDLRender::scrollRight() {
    for(uint8_t y = 0; y < mHeight; y++) {
        memmove(&mPixelData[y][4], &mPixelData[y][0],124 *4);
        memset(&mPixelData[y][0], 0, 4*4);
    }
}

void SDLRender::beep(uint8_t dur) {
}

uint8_t SDLRender::random() {
    return rand();
}

uint16_t SDLRender::buttons() {
    return mButtons;
}
