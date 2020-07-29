#include <SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void vertical_line(
        uint32_t *const surface,
        uint32_t x,
        uint32_t y1, uint32_t y2,
        uint32_t color) {
    // Border.
    surface[y1 * WINDOW_WIDTH + x] = 0;
    surface[y2 * WINDOW_WIDTH + x] = 0;

    // Fill.
    const uint32_t y_min = MIN(y1, y2);
    const uint32_t y_max = MAX(y1, y2);
    for (uint32_t y = y_min + 1; y < y_max; ++y) {
        surface[y * WINDOW_WIDTH + x] = color;
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Failed to initialize SDL.\n");
        return 1;
    }
    atexit(SDL_Quit);

    SDL_Window *const window = SDL_CreateWindow(
            "Keter",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
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

    SDL_Texture *const texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH, WINDOW_HEIGHT);
    if (texture == NULL) {
        fprintf(stderr, "Failed to create SDL texture.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_Delay(1000);

    uint32_t *const pixels = calloc(1, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
    if (pixels == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data.\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    for (uint32_t i = 30; i < WINDOW_HEIGHT - 30; ++i) {
        for (uint32_t j = 30; j < WINDOW_WIDTH - 30; ++j) {
            uint32_t pixel = rand() % 255;
            pixel += rand() % 255 << 8;
            pixel += rand() % 255 << 16;
            pixel += rand() % 255 << 24;
            pixels[i * WINDOW_WIDTH + j] = pixel;
        }
    }

    vertical_line(pixels, 60, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFFFF0000);
    vertical_line(pixels, 61, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFFFF0000);
    vertical_line(pixels, 62, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF00FF00);
    vertical_line(pixels, 63, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF00FF00);
    vertical_line(pixels, 64, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF0000FF);
    vertical_line(pixels, 65, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF0000FF);
    vertical_line(pixels, 66, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF000000);
    vertical_line(pixels, 67, WINDOW_HEIGHT / 3, WINDOW_HEIGHT / 3 * 2, 0xFF000000);

    SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(2000);
    free(pixels);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
