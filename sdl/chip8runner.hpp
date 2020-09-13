#pragma once

#include "SDL2/SDL.h"
#include "render.hpp"
#include "tracer.hpp"
#include "../src/chip8/simplemem.hpp"
#include "../src/chip8/chip8.hpp"
#include <vector>

struct RunnerProgram {
    const uint8_t *code;
    uint16_t size;
    const char* name;
};

class Chip8Runner {
    void pollEvents();

    SimpleMemory mMemory;
    SDL_Renderer *mSDL_Renderer;
    SDLRender mRender;
    //ConsoleTracer mTracer;
    NopTracer mTracer;
    Chip8 mEmu;
    std::vector<RunnerProgram> mPrograms;
    uint8_t mProgramIndex = 0;
    
    void handleKeyEvent(SDL_Scancode code, bool pressed);
    bool tick();
    void nextProgram();
    void prevProgram();
    void loadEmu();

    public:
    Chip8Runner(SDL_Renderer *renderer, std::vector<RunnerProgram> programs);
    ~Chip8Runner();
    void run();
};
