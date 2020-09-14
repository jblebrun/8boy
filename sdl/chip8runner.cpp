#include "chip8runner.hpp"
#include "render.hpp"
#include <chrono>
#include <thread>

#define EMU_STEP_DELAY 1000

Chip8Runner::Chip8Runner(SDL_Renderer *renderer, std::vector<RunnerProgram> programs) :
    mPrograms(programs),
    mSDL_Renderer(renderer), 
    mRender(renderer),
    mEmu(mRender, mMemory, mTracer) { }

Chip8Runner::~Chip8Runner() {
}

void Chip8Runner::loadEmu() {
    RunnerProgram& pgm = mPrograms[mProgramIndex];
    printf("Running %s\n", pgm.name);
    mMemory.load(pgm.code, pgm.size);
    mRender.clear();
    mEmu.Reset();
}


void Chip8Runner::nextProgram() {
    mProgramIndex++;
    if(mProgramIndex >= mPrograms.size()) mProgramIndex = 0;
    loadEmu();
}

void Chip8Runner::prevProgram() {
    mProgramIndex--;
    if(mProgramIndex >= mPrograms.size()) mProgramIndex = mPrograms.size()-1;
    loadEmu();
}

void Chip8Runner::handleKeyEvent(SDL_Scancode code, bool pressed) {
    switch(code) {
        case SDL_SCANCODE_1: return mRender.setKeyState(1, pressed);
        case SDL_SCANCODE_2: return mRender.setKeyState(2, pressed);
        case SDL_SCANCODE_3: return mRender.setKeyState(3, pressed);
        case SDL_SCANCODE_4: return mRender.setKeyState(0xC, pressed);
        case SDL_SCANCODE_Q: return mRender.setKeyState(4, pressed);
        case SDL_SCANCODE_W: return mRender.setKeyState(5, pressed);
        case SDL_SCANCODE_E: return mRender.setKeyState(6, pressed);
        case SDL_SCANCODE_R: return mRender.setKeyState(0xD, pressed);
        case SDL_SCANCODE_A: return mRender.setKeyState(7, pressed);
        case SDL_SCANCODE_S: return mRender.setKeyState(8, pressed);
        case SDL_SCANCODE_D: return mRender.setKeyState(9, pressed);
        case SDL_SCANCODE_F: return mRender.setKeyState(0xE, pressed);
        case SDL_SCANCODE_Z: return mRender.setKeyState(0xA, pressed);
        case SDL_SCANCODE_X: return mRender.setKeyState(0, pressed);
        case SDL_SCANCODE_C: return mRender.setKeyState(0xB, pressed);
        case SDL_SCANCODE_V: return mRender.setKeyState(0xF, pressed);
        case SDL_SCANCODE_LEFT: if(pressed) return prevProgram(); else return;
        case SDL_SCANCODE_RIGHT: if(pressed) return nextProgram(); else return;
        default: return;
    }
}

bool Chip8Runner::tick() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE: return false; 
                }
                break;
            case SDL_KEYDOWN:  
            case SDL_KEYUP:
                if(!event.key.repeat) {
                    handleKeyEvent(event.key.keysym.scancode, event.key.state == SDL_PRESSED);
                }
                break;
            case SDL_QUIT: return false;
        }
    }
    mEmu.Tick();
    return true;
}

void Chip8Runner::run() {
    loadEmu();
    pollEvents();
}

void Chip8Runner::pollEvents() {
    uint32_t lastEmu = SDL_GetTicks();
    uint32_t lastRender = SDL_GetTicks();
    while(1) {
        uint32_t ticks = SDL_GetTicks();
        mEmu.Step();
        std::this_thread::sleep_for (std::chrono::microseconds(EMU_STEP_DELAY));
        if (ticks-lastRender > 16) {
            lastRender = ticks;
            if(!tick()) {
                return;
            }
        }
    }
}
