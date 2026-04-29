#pragma once

#include "vector.h"

#define CAMERA_H

typedef  struct {
    vec3_t position;
    vec3_t direction;
} camera_t;

extern camera_t camera; //TODO: dont do this



