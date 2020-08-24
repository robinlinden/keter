#include "keter/draw.h"
#include "keter/math.h"
#include "keter/player.h"

#include <SDL.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(int argc, char *argv[]) {
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

    SDL_SetRelativeMouseMode(SDL_ENABLE);

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

    uint32_t *const first_person_surface = calloc(1, VIEWPORT_BYTES);
    if (first_person_surface == NULL) {
        fprintf(stderr, "Failed to allocate for pixel data 2.\n");
        free(relative_surface);
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

    const SDL_Rect first_person_viewport = {
        .x = 0,
        .y = VIEWPORT_HEIGHT,
        .w = VIEWPORT_WIDTH,
        .h = VIEWPORT_HEIGHT,
    };

    bool wasd[4] = {0}; // Held keys.
    Player p = {{.x = 100, .y = 100}};

    const int32_t walls[] = {
            300, 100, 300, 400,
            300, 400, 250, 400,
            250, 400, 250, 100,
            250, 100, 300, 100,

            50, 50, 50, 100,
            50, 100, 100, 100,
            100, 100, 100, 50,
            100, 50, 50, 50
    };

    bool running = true;
    while (running) {
        // Absolute view
        memset(absolute_surface, 0, VIEWPORT_BYTES);
        draw_player(absolute_surface, &p);

        for (size_t i = 0; i < sizeof(walls) / sizeof(walls[0]); i += 4) {
            draw_line(absolute_surface, walls[i], walls[i+1], walls[i+2], walls[i+3], 0x0000FFFF);
        }

        SDL_UpdateTexture(texture, &absolute_viewport, absolute_surface, VIEWPORT_STRIDE);

        // Relative view
        memset(relative_surface, 0x22, VIEWPORT_BYTES);
        draw_relative_player(relative_surface);

        for (size_t i = 0; i < sizeof(walls) / sizeof(walls[0]); i += 4) {
            draw_relative_line(relative_surface, &p, walls[i], walls[i+1], walls[i+2], walls[i+3], 0x0000FFFF);
        }

        SDL_UpdateTexture(texture, &relative_viewport, relative_surface, VIEWPORT_STRIDE);

        // First-person view
        memset(first_person_surface, 0x44, VIEWPORT_BYTES);

        draw_wall(first_person_surface, &p, 250, 400, 250, 100, 0x0000FFFF);

        SDL_UpdateTexture(texture, &first_person_viewport, first_person_surface, VIEWPORT_STRIDE);

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

        int mouse_x, mouse_y;
        SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
        p.heading += (float)mouse_x * 0.02f;

        const bool forward = wasd[0];
        const bool left = wasd[1];
        const bool backward = wasd[2];
        const bool right = wasd[3];
        const bool key_held = forward || left || backward || right;
        Vector2 move_vec = {0};
        move_vec.x += forward ? cosf(p.heading) : 0;
        move_vec.y += forward ? sinf(p.heading) : 0;
        move_vec.x -= backward ? cosf(p.heading) : 0;
        move_vec.y -= backward ? sinf(p.heading) : 0;
        move_vec.x += left ? sinf(p.heading) : 0;
        move_vec.y -= left ? cosf(p.heading) : 0;
        move_vec.x -= right ? sinf(p.heading) : 0;
        move_vec.y += right ? cosf(p.heading) : 0;

        const float acceleration = key_held ? 0.4f : 0.2f;
        p.vel.x = p.vel.x * (1 - acceleration) + move_vec.x * acceleration;
        p.vel.y = p.vel.y * (1 - acceleration) + move_vec.y * acceleration;
        p.pos.x += p.vel.x;
        p.pos.y += p.vel.y;

        if (p.heading > M_PI * 2) {
            p.heading -= M_PI * 2;
        } else if (p.heading < 0) {
            p.heading += M_PI * 2;
        }

        SDL_Delay(10);
    }

    free(first_person_surface);
    free(relative_surface);
    free(absolute_surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
