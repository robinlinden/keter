#ifndef KETER_VIEWPORT_H
#define KETER_VIEWPORT_H

#include <stdint.h>

#define VIEWPORT_WIDTH 400
#define VIEWPORT_HEIGHT 300
#define VIEWPORT_BYTES VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t)
#define VIEWPORT_STRIDE VIEWPORT_WIDTH * sizeof(uint32_t)

#endif
