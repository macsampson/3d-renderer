#pragma once

#include "matrix.h"
#include "triangle.h"
#include <stdbool.h>

#define MAX_SKYBOX_TRIANGLES 64

// face order: right (+X), left (-X), top (+Y), bottom (-Y), front (+Z), back (-Z)
void init_skybox(
    const char* right, const char* left,
    const char* top,   const char* bottom,
    const char* front, const char* back
);

void process_skybox(
    mat4_t proj_matrix,
    mat4_t view_matrix,
    triangle_t* out_triangles,
    int* num_out
);

bool is_skybox_initialized(void);
