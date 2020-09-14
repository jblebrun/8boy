#include "SDL2/SDL.h"
#include <stdio.h>
#include "chip8runner.hpp"
#define PROGMEM
#include "programs.h"
#include <thread>

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);       

    SDL_Window *window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        10,           // initial x position
        10,           // initial y position
        640,                               // width, in pixels
        320,                               // height, in pixels
        SDL_WINDOW_RESIZABLE    // flags - see below
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    std::vector<RunnerProgram> pgms(PROGRAM_COUNT);
    for(int i = 0; i < PROGRAM_COUNT; i++) {
        const Program* pgm = &programs[i];
        pgms[i] = (RunnerProgram){pgm->code, pgm->size, pgm->name};
    }

    Chip8Runner runner(renderer, pgms);
    runner.run();

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
