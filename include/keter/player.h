#ifndef KETER_PLAYER_H
#define KETER_PLAYER_H

#include "keter/math.h"

#include <math.h>

typedef struct Player {
    float x, y;
    float heading;
} Player;

static inline void make_relative(const Player *const p, float *const x, float *const y) {
    const float angle = -M_PI / 2 - p->heading;
    const float xt = (*x - p->x) * cosf(angle) - (*y - p->y) * sinf(angle);
    const float yt = (*x - p->x) * sinf(angle) + (*y - p->y) * cosf(angle);
    *x = xt;
    *y = yt;
}

#endif
