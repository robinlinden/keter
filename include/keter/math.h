#ifndef KETER_MATH_H
#define KETER_MATH_H

#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static inline float cross(float x0, float y0, float x1, float y1) {
    return x0 * y1 - y0 * x1;
}

static inline bool intersect(
        float *x, float *y,
        float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
    const float a = cross(x0, y0, x1, y1);
    const float b = cross(x2, y2, x3, y3);
    const float d = cross(x0 - x1, y0 - y1, x2 - x3, y2 - y3);
    if (d == 0) {
        return false;
    }

    *x = cross(a, x0 - x1, b, x2 - x3) / d;
    *y = cross(a, y0 - y1, b, y2 - y3) / d;
    return true;
}

#endif
