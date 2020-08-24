#ifndef KETER_DRAW_H
#define KETER_DRAW_H

#include "keter/math.h"
#include "keter/player.h"
#include "keter/viewport.h"

#include <math.h>
#include <stdint.h>

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
    const float step = MAX(fabsf(dx), fabsf(dy));
    dx /= step;
    dy /= step;

    float x = x0;
    float y = y0;

    for (uint32_t i = 0; i < step; ++i, x += dx, y += dy) {
        draw_pixel(surface, x, y, color);
    }
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

static void draw_relative_player(uint32_t *const surface) {
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
    const uint32_t y_min = MIN(y1, y2);
    const uint32_t y_max = MAX(y1, y2);
    for (uint32_t y = y_min; y < y_max; ++y) {
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

static void draw_wall(
        uint32_t *const surface,
        const Player *const p,
        float x0, float y0,
        float x1, float y1,
        int32_t color) {
    make_relative(p, &x0, &y0);
    make_relative(p, &x1, &y1);

    float z0 = -y0;
    float z1 = -y1;

    // Is the object fully behind the player?
    if (z0 <= 0 && z1 <= 0) {
        return;
    }

    // Is the object partially behind the player?
    if (z0 <= 0 || z1 <= 0) {
        const float nearz = 1e-4f, farz = 5, nearx = 1e-5f, farx = 20.f;

        // Find an intersection between the wall and the approximate edges of player's view
        float lx, ly;
        float rx, ry;
        intersect(&lx, &ly, x0, z0, x1, z1, -nearx, nearz, -farx, farz);
        intersect(&rx, &ry, x0, z0, x1, z1, nearx, nearz, farx, farz);

        if (z0 < nearz) {
            if (ly > 0) {
                x0 = lx;
                y0 = z0 = ly;
            } else {
                x0 = rx;
                y0 = z0 = ry;
            }
            y0 = -y0;
        } else if (z1 < nearz) {
            if (ly > 0) {
                x1 = lx;
                y1 = z1 = ly;
            } else {
                x1 = rx;
                y1 = z1 = ry;
            }
            y1 = -y1;
        }
    }

    x0 = -x0 * 128 / y0;
    float y0a = -2000 / y0;
    float y0b = 2000 / y0;

    x1 = -x1 * 128 / y1;
    float y1a = -2000 / y1;
    float y1b = 2000 / y1;

    x0 += VIEWPORT_WIDTH / 2;
    x1 += VIEWPORT_WIDTH / 2;
    y0a += VIEWPORT_HEIGHT / 2;
    y0b += VIEWPORT_HEIGHT / 2;
    y1a += VIEWPORT_HEIGHT / 2;
    y1b += VIEWPORT_HEIGHT / 2;

    draw_line(surface, x0, y0a, x1, y1a, color); // top
    draw_line(surface, x0, y0b, x1, y1b, color); // bottom
    draw_line(surface, x0, y0a, x0, y0b, color); // left
    draw_line(surface, x1, y1a, x1, y1b, color); // right
}

#endif
