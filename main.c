#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Failed to initialize SDL.\n");
        return 1;
    }
    atexit(SDL_Quit);

    SDL_Window *const window = SDL_CreateWindow(
            "Keter",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 480,
            SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Failed to create SDL window.\n");
        return 1;
    }

    SDL_Renderer *const renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        fprintf(stderr, "Failed to create SDL renderer.\n");
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(2000);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
