#ifndef KETER_PLAYER_H
#define KETER_PLAYER_H

#include "keter/math.h"
#include "keter/point.h"
#include "keter/vector.h"

#include <math.h>

typedef struct Player {
    Point2 pos;
    Vector2 vel;
    float heading;
} Player;

static inline void make_relative(const Player *const p, float *const x, float *const y) {
    const float angle = -M_PI / 2 - p->heading;
    const float xt = (*x - p->pos.x) * cosf(angle) - (*y - p->pos.y) * sinf(angle);
    const float yt = (*x - p->pos.x) * sinf(angle) + (*y - p->pos.y) * cosf(angle);
    *x = xt;
    *y = yt;
}

#endif
