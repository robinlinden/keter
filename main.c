#include <SDL.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEWPORT_WIDTH 400
#define VIEWPORT_HEIGHT 600

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(a, min, max) MIN(MAX((a), (min)), (max))

typedef struct Player {
    float x, y;
    float heading;
} Player;

static inline void draw_pixel(uint32_t *const surface, int32_t x, int32_t y, uint32_t color) {
    surface[CLAMP(y, 0, VIEWPORT_HEIGHT - 1) * VIEWPORT_WIDTH + CLAMP(x, 0, VIEWPORT_WIDTH - 1)] = color;
}

static void draw_line(
        uint32_t *const surface,
        int32_t x0, int32_t y0,
        int32_t x1, int32_t y1,
        int32_t color) {
    x0 = CLAMP(x0, 0, VIEWPORT_WIDTH - 1);
    x1 = CLAMP(x1, 0, VIEWPORT_WIDTH - 1);
    y0 = CLAMP(y0, 0, VIEWPORT_HEIGHT - 1);
    y1 = CLAMP(y1, 0, VIEWPORT_HEIGHT - 1);
    const int32_t x_min = MIN(x0, x1);
    const int32_t x_max = MAX(x0, x1);
    const int32_t y_min = MIN(y0, y1);
    const int32_t y_max = MAX(y0, y1);

    float dx = x_max - x_min;
    float dy = y_max - y_min;
    const float step = MAX(dx, dy);
    dx /= step;
    dy /= step;

    float x = x_min;
    float y = y_min;

    // Fill.
    for (uint32_t i = 0; i < step; ++i, x += dx, y += dy) {
        draw_pixel(surface, x, y, color);
    }

    // Border.
    draw_pixel(surface, x0, y0, 0);
    draw_pixel(surface, x1, y1, 0);
}

static void draw_vertical_line(
        uint32_t *const surface,
        uint32_t x,
        uint32_t y1, uint32_t y2,
        uint32_t color) {
    // Border.
    draw_pixel(surface, x, y1, 0);
    draw_pixel(surface, x, y2, 0);

    // Fill.
    const uint32_t y_min = MIN(y1, y2);
    const uint32_t y_max = MAX(y1, y2);
    for (uint32_t y = y_min + 1; y < y_max; ++y) {
        draw_pixel(surface, x, y, color);
    }
}

static void draw_player(
        uint32_t *const surface,
        const Player *const p) {
    for (uint32_t y = p->y - 5; y < p->y + 5; ++y) {
        for (uint32_t x = p->x - 5; x < p->x + 5; ++x) {
            draw_pixel(surface, x, y, 0x00FF0000);
        }
    }

    const uint32_t heading_y = p->y + sinf(p->heading) * 15;
    const uint32_t heading_x = p->x + cosf(p->heading) * 15;
    draw_pixel(surface, heading_x, heading_y, 0x00FF00FF);
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

    SDL_Texture *const texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH, WINDOW_HEIGHT);
    if (texture == NULL) {
        fprintf(stderr, "Failed to create SDL texture 0.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    uint32_t *const pixels0 = calloc(1, VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t));
    if (pixels0 == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data 0.\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    uint32_t *const pixels1 = calloc(1, VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t));
    if (pixels1 == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data 1.\n");
        free(pixels0);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_Rect viewport0 = {
        .x = 0,
        .y = 0,
        .w = 400,
        .h = 600,
    };

    SDL_Rect viewport1 = {
        .x = 400,
        .y = 0,
        .w = 400,
        .h = 600,
    };

    bool wasd[4] = {0}; // Held keys.
    Player p = { .x = 100, .y = 100 };

    bool running = true;
    while (running) {
        memset(pixels0, 0, VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t));
        draw_player(pixels0, &p);
        draw_vertical_line(pixels0, 300, 100, 400, 0x0000FFFF);
        SDL_UpdateTexture(texture, &viewport0, pixels0, VIEWPORT_WIDTH * sizeof(uint32_t));

        memset(pixels1, 0xCD, VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t));
        draw_vertical_line(pixels1, 100, 30, VIEWPORT_HEIGHT - 30, 0x00FF00FF);
        draw_line(pixels1, 10, 10, 100, 200, 0x00FF00FF);
        SDL_UpdateTexture(texture, &viewport1, pixels1, VIEWPORT_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch(ev.key.keysym.sym) {
                    case 'w':
                        wasd[0] = ev.type == SDL_KEYDOWN;
                        break;
                    case 'a':
                        wasd[1] = ev.type == SDL_KEYDOWN;
                        break;
                    case 's':
                        wasd[2] = ev.type == SDL_KEYDOWN;
                        break;
                    case 'd':
                        wasd[3] = ev.type == SDL_KEYDOWN;
                        break;
                    default: break;
                }
                break;
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        p.x += wasd[0] ? cosf(p.heading) : 0;
        p.y += wasd[0] ? sinf(p.heading) : 0;
        p.x -= wasd[2] ? cosf(p.heading) : 0;
        p.y -= wasd[2] ? sinf(p.heading) : 0;

        p.heading -= wasd[1] ? 0.1 : 0;
        p.heading += wasd[3] ? 0.1 : 0;
        if (p.heading > M_PI * 2) {
            p.heading -= M_PI * 2;
        } else if (p.heading < 0) {
            p.heading += M_PI * 2;
        }

        SDL_Delay(10);
    }

    free(pixels1);
    free(pixels0);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
