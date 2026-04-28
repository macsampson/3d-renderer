# 3D Renderer — Top 3 FPS Optimizations

## 1. Add compiler optimization flags (`Makefile:2`)

**Estimated impact: 2–10x FPS increase — zero code changes required.**

```makefile
# Current
gcc -Wall -std=c99 ./src/*.c -lSDL2 -lm -o renderer

# Fix
gcc -O3 -march=native -flto -Wall -std=c99 ./src/*.c -lSDL2 -lm -o renderer
```

The renderer runs with zero compiler optimizations. `-O3` enables auto-vectorization, loop unrolling, and inlining. `-march=native` unlocks SIMD (SSE/AVX). `-flto` enables cross-file inlining of the small math functions called millions of times per frame.

---

## 2. Move world matrix construction outside the vertex loop (`main.c:162–175`)

**Estimated impact: ~3x reduction in matrix work per frame.**

The 5-matrix world matrix is rebuilt inside the inner loop over 3 vertices — so it's computed 3× per triangle, every frame. All 5 component matrices (`scale_matrix`, `rotation_matrix_*`, `translation_matrix`) are already computed above the loop. Move lines 168–173 outside the `for (int j = 0; j < 3; j++)` loop:

```c
// Build ONCE per face, before the vertex loop
mat4_t world_matrix = mat4_identity();
world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

for (int j = 0; j < 3; j++) {
    vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
    transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
    // ...
}
```

Since the world matrix is identical for the whole mesh every frame, it can also be moved above the face loop entirely for maximum gain.

---

## 3. Pre-compute per-triangle values instead of per-pixel divisions (`triangle.c:282–340`)

**Estimated impact: largest gain on filled/textured rendering — divisions are ~15x slower than multiplies.**

`barycentric_weights()` computes `area_parallelogram_abc` (constant per triangle) and performs 2 divisions every pixel. Additionally, `1/point_a.w`, `1/point_b.w`, `1/point_c.w` are recomputed every pixel in `draw_triangle_pixel` (lines 328–329) and `draw_texel` (lines 376–377).

Compute these once per triangle and pass them in:

```c
// Pre-compute per triangle (before the scanline loops)
float inv_w_a = 1.0f / point_a.w;
float inv_w_b = 1.0f / point_b.w;
float inv_w_c = 1.0f / point_c.w;
float area = (ac.x * ab.y - ac.y * ab.x); // move out of barycentric_weights
float inv_area = 1.0f / area;              // one division reused every pixel
```

Then inside the pixel loop, multiply by `inv_area` instead of dividing by `area`.

---

## Recommended Order

| Priority | Change | Effort | Gain |
|----------|--------|--------|------|
| 1 | Add `-O3 -march=native -flto` to Makefile | 1 line | 2–10x |
| 2 | Move world matrix build outside vertex loop | 5-line move | ~3x on transform |
| 3 | Pre-compute `inv_area` and `1/w` per triangle | Refactor function signatures | Largest gain on fill/texture |

Together these should deliver **5–20x FPS improvement** before touching SIMD or multithreading.
