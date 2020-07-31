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
#define VIEWPORT_BYTES VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t)
#define VIEWPORT_STRIDE VIEWPORT_WIDTH * sizeof(uint32_t)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct Player {
    float x, y;
    float heading;
} Player;

static inline void draw_pixel(uint32_t *const surface, int32_t x, int32_t y, uint32_t color) {
    if (y < 0 || y > VIEWPORT_HEIGHT - 1) return;
    if (x < 0 || x > VIEWPORT_WIDTH - 1) return;
    surface[y * VIEWPORT_WIDTH + x] = color;
}

static void draw_line(
        uint32_t *const surface,
        int32_t x0, int32_t y0,
        int32_t x1, int32_t y1,
        int32_t color) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    const float step = MAX(fabs(dx), fabs(dy));
    dx /= step;
    dy /= step;

    float x = x0;
    float y = y0;

    // Fill.
    for (uint32_t i = 0; i < step; ++i, x += dx, y += dy) {
        draw_pixel(surface, x, y, color);
    }

    // Border.
    draw_pixel(surface, x0, y0, 0);
    draw_pixel(surface, x1, y1, 0);
}

static inline void make_relative(const Player *const p, float *const x, float *const y) {
    const float angle = -M_PI / 2 - p->heading;
    const float xt = (*x - p->x) * cosf(angle) - (*y - p->y) * sinf(angle);
    const float yt = (*x - p->x) * sinf(angle) + (*y - p->y) * cosf(angle);
    *x = xt;
    *y = yt;
}

static void draw_relative_line(
        uint32_t *const surface,
        const Player *const p,
        float x0, float y0,
        float x1, float y1,
        int32_t color) {
    make_relative(p, &x0, &y0);
    make_relative(p, &x1, &y1);
    draw_line(
            surface,
            x0 + VIEWPORT_WIDTH / 2, y0 + VIEWPORT_HEIGHT / 2,
            x1 + VIEWPORT_WIDTH / 2, y1 + VIEWPORT_HEIGHT / 2,
            color);
}

static void draw_relative_player(
        uint32_t *const surface,
        const Player *const p) {
    for (uint32_t y = VIEWPORT_HEIGHT / 2 - 5; y < VIEWPORT_HEIGHT / 2 + 5; ++y) {
        for (uint32_t x = VIEWPORT_WIDTH / 2 - 5; x < VIEWPORT_WIDTH / 2 + 5; ++x) {
            draw_pixel(surface, x, y, 0x00FF0000);
        }
    }

    const uint32_t heading_x = VIEWPORT_WIDTH / 2;
    const uint32_t heading_y = VIEWPORT_HEIGHT / 2 - 10;
    draw_pixel(surface, heading_x, heading_y, 0x00FF00FF);
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

    uint32_t *const absolute_surface = calloc(1, VIEWPORT_BYTES);
    if (absolute_surface == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data 0.\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    uint32_t *const relative_surface = calloc(1, VIEWPORT_BYTES);
    if (relative_surface == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data 1.\n");
        free(absolute_surface);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    const SDL_Rect absolute_viewport = {
        .x = 0,
        .y = 0,
        .w = VIEWPORT_WIDTH,
        .h = VIEWPORT_HEIGHT,
    };

    const SDL_Rect relative_viewport = {
        .x = VIEWPORT_WIDTH,
        .y = 0,
        .w = VIEWPORT_WIDTH,
        .h = VIEWPORT_HEIGHT,
    };

    bool wasd[4] = {0}; // Held keys.
    Player p = { .x = 100, .y = 100 };

    bool running = true;
    while (running) {
        // Absolute view
        memset(absolute_surface, 0, VIEWPORT_BYTES);
        draw_player(absolute_surface, &p);

        draw_line(absolute_surface, 300, 100, 300, 400, 0x0000FFFF);
        draw_line(absolute_surface, 300, 400, 250, 400, 0x0000FFFF);
        draw_line(absolute_surface, 250, 400, 250, 100, 0x0000FFFF);
        draw_line(absolute_surface, 250, 100, 300, 100, 0x0000FFFF);

        draw_line(absolute_surface, 50, 50, 50, 100, 0x0000FFFF);
        draw_line(absolute_surface, 50, 100, 100, 100, 0x0000FFFF);
        draw_line(absolute_surface, 100, 100, 100, 50, 0x0000FFFF);
        draw_line(absolute_surface, 100, 50, 50, 50, 0x0000FFFF);

        SDL_UpdateTexture(texture, &absolute_viewport, absolute_surface, VIEWPORT_STRIDE);

        // Relative view
        memset(relative_surface, 0x22, VIEWPORT_BYTES);
        draw_relative_player(relative_surface, &p);

        draw_relative_line(relative_surface, &p, 300, 100, 300, 400, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 300, 400, 250, 400, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 250, 400, 250, 100, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 250, 100, 300, 100, 0x0000FFFF);

        draw_relative_line(relative_surface, &p, 50, 50, 50, 100, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 50, 100, 100, 100, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 100, 100, 100, 50, 0x0000FFFF);
        draw_relative_line(relative_surface, &p, 100, 50, 50, 50, 0x0000FFFF);

        SDL_UpdateTexture(texture, &relative_viewport, relative_surface, VIEWPORT_STRIDE);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch(ev.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        wasd[0] = ev.type == SDL_KEYDOWN;
                        break;
                    case SDL_SCANCODE_A:
                        wasd[1] = ev.type == SDL_KEYDOWN;
                        break;
                    case SDL_SCANCODE_S:
                        wasd[2] = ev.type == SDL_KEYDOWN;
                        break;
                    case SDL_SCANCODE_D:
                        wasd[3] = ev.type == SDL_KEYDOWN;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        running = false;
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

    free(relative_surface);
    free(absolute_surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
