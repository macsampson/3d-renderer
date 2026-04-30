#pragma once

#include "vector.h"

#define CAMERA_H

typedef struct {
    vec3_t position;
    vec3_t direction;
    vec3_t forward_velocity;
    float yaw_angle;
} camera_t;

extern camera_t camera; //TODO: dont do this



