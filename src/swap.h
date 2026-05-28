#pragma once

#define SWAP_H

static inline void int_swap(int* a, int* b) {
    int tmp = *a; *a = *b; *b = tmp;
}
static inline void float_swap(float* a, float* b) {
    float tmp = *a; *a = *b; *b = tmp;
}
