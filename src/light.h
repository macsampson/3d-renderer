#pragma once

#include "vector.h"
#include <stdint.h>
#define LIGHT_H

typedef struct {
    vec3_t dir;
    uint32_t color;
} light_t;

void init_light(vec3_t dir);
vec3_t get_light_dir();

uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);