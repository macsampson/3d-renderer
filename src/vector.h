#pragma once

#define VECTOR_H

#include <math.h>

typedef struct {
    float x, y;
} vec2_t;

typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    float x, y, z, w;
} vec4_t;

static inline vec2_t vec2_new(float x, float y) {
    vec2_t r = {x, y}; return r;
}
static inline float vec2_length(vec2_t v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}
static inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t r = {a.x + b.x, a.y + b.y}; return r;
}
static inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    vec2_t r = {a.x - b.x, a.y - b.y}; return r;
}
static inline vec2_t vec2_mult(vec2_t v, float f) {
    vec2_t r = {v.x * f, v.y * f}; return r;
}
static inline vec2_t vec2_div(vec2_t v, float f) {
    vec2_t r = {v.x / f, v.y / f}; return r;
}
static inline float vec2_dot(vec2_t a, vec2_t b) {
    return a.x * b.x + a.y * b.y;
}
static inline void vec2_normalize(vec2_t* v) {
    float len = sqrtf(v->x * v->x + v->y * v->y);
    v->x /= len; v->y /= len;
}

static inline vec3_t vec3_new(float x, float y, float z) {
    vec3_t r = {x, y, z}; return r;
}
static inline float vec3_length(vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    vec3_t r = {a.x + b.x, a.y + b.y, a.z + b.z}; return r;
}
static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    vec3_t r = {a.x - b.x, a.y - b.y, a.z - b.z}; return r;
}
static inline vec3_t vec3_mult(vec3_t v, float f) {
    vec3_t r = {v.x * f, v.y * f, v.z * f}; return r;
}
static inline vec3_t vec3_div(vec3_t v, float f) {
    vec3_t r = {v.x / f, v.y / f, v.z / f}; return r;
}
static inline vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t r = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return r;
}
static inline float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline void vec3_normalize(vec3_t* v) {
    float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= len; v->y /= len; v->z /= len;
}
static inline vec3_t vec3_clone(vec3_t* v) {
    vec3_t r = {v->x, v->y, v->z}; return r;
}

static inline vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t r = {v.x, v.y * cosf(angle) - v.z * sinf(angle), v.y * sinf(angle) + v.z * cosf(angle)};
    return r;
}
static inline vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t r = {v.x * cosf(angle) - v.z * sinf(angle), v.y, v.x * sinf(angle) + v.z * cosf(angle)};
    return r;
}
static inline vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t r = {v.x * cosf(angle) - v.y * sinf(angle), v.x * sinf(angle) + v.y * cosf(angle), v.z};
    return r;
}

static inline vec2_t vec2_from_vec4(vec4_t v) {
    vec2_t r = {v.x, v.y}; return r;
}
static inline vec3_t vec3_from_vec4(vec4_t v) {
    vec3_t r = {v.x, v.y, v.z}; return r;
}
static inline vec4_t vec4_from_vec3(vec3_t v) {
    vec4_t r = {v.x, v.y, v.z, 1.0f}; return r;
}
