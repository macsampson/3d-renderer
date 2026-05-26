#include "skybox.h"
#include "clipping.h"
#include "display.h"
#include "matrix.h"
#include "upng.h"
#include "vector.h"
#include <stdbool.h>

// Inward-facing unit cube at ±10
static const vec3_t CUBE_VERTS[8] = {
    {-10, -10, -10}, // 0
    { 10, -10, -10}, // 1
    { 10,  10, -10}, // 2
    {-10,  10, -10}, // 3
    {-10, -10,  10}, // 4
    { 10, -10,  10}, // 5
    { 10,  10,  10}, // 6
    {-10,  10,  10}, // 7
};

typedef struct {
    int a, b, c;
    tex2_t a_uv, b_uv, c_uv;
    int face_idx; // 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
} skybox_tri_def_t;

// Winding gives inward-facing normals (cross(B-A, C-A) points toward cube center).
// UVs are defined looking at each face from outside (standard cubemap orientation).
// U increases left-to-right, V increases bottom-to-top from outside.
static const skybox_tri_def_t CUBE_TRIS[12] = {
    // +X right  (verts 1,5,6,2)
    {1, 5, 6, {1, 0}, {0, 0}, {0, 1}, 0},
    {1, 6, 2, {1, 0}, {0, 1}, {1, 1}, 0},
    // -X left   (verts 0,4,7,3)
    {0, 3, 7, {0, 0}, {0, 1}, {1, 1}, 1},
    {0, 7, 4, {0, 0}, {1, 1}, {1, 0}, 1},
    // +Y top    (verts 3,2,6,7)
    {3, 2, 6, {0, 1}, {1, 1}, {1, 0}, 2},
    {3, 6, 7, {0, 1}, {1, 0}, {0, 0}, 2},
    // -Y bottom (verts 0,1,5,4)
    {0, 4, 5, {0, 0}, {0, 1}, {1, 1}, 3},
    {0, 5, 1, {0, 0}, {1, 1}, {1, 0}, 3},
    // +Z front  (verts 4,5,6,7)
    {4, 7, 6, {0, 0}, {0, 1}, {1, 1}, 4},
    {4, 6, 5, {0, 0}, {1, 1}, {1, 0}, 4},
    // -Z back   (verts 0,1,2,3)
    {0, 1, 3, {1, 0}, {0, 0}, {1, 1}, 5},
    {1, 2, 3, {0, 0}, {0, 1}, {1, 1}, 5},
};

static upng_t* face_textures[6];
static bool skybox_ready = false;

static upng_t* load_face_png(const char* path) {
    upng_t* png = upng_new_from_file(path);
    if (png == NULL) return NULL;
    upng_decode(png);
    upng_ensure_rgba8(png);  // expand RGB8 → RGBA8 so draw_texel's uint32_t cast is valid
    if (upng_get_error(png) != UPNG_EOK) {
        upng_free(png);
        return NULL;
    }
    return png;
}

void init_skybox(
    const char* right, const char* left,
    const char* top,   const char* bottom,
    const char* front, const char* back
) {
    face_textures[0] = load_face_png(right);
    face_textures[1] = load_face_png(left);
    face_textures[2] = load_face_png(top);
    face_textures[3] = load_face_png(bottom);
    face_textures[4] = load_face_png(front);
    face_textures[5] = load_face_png(back);

    for (int i = 0; i < 6; i++) {
        if (face_textures[i] != NULL) {
            skybox_ready = true;
            break;
        }
    }
}

bool is_skybox_initialized(void) {
    return skybox_ready;
}

void process_skybox(
    mat4_t proj_matrix,
    mat4_t view_matrix,
    triangle_t* out_triangles,
    int* num_out
) {
    *num_out = 0;

    // Strip translation so the skybox stays centered on the camera regardless of position.
    mat4_t sky_view = view_matrix;
    sky_view.m[0][3] = 0.0f;
    sky_view.m[1][3] = 0.0f;
    sky_view.m[2][3] = 0.0f;

    for (int i = 0; i < 12; i++) {
        const skybox_tri_def_t* tri = &CUBE_TRIS[i];

        vec4_t va = mat4_mul_vec4(sky_view, vec4_from_vec3(CUBE_VERTS[tri->a]));
        vec4_t vb = mat4_mul_vec4(sky_view, vec4_from_vec3(CUBE_VERTS[tri->b]));
        vec4_t vc = mat4_mul_vec4(sky_view, vec4_from_vec3(CUBE_VERTS[tri->c]));

        polygon_t poly = create_polygon_from_triangle(
            vec3_from_vec4(va), vec3_from_vec4(vb), vec3_from_vec4(vc),
            tri->a_uv, tri->b_uv, tri->c_uv
        );
        clip_polygon(&poly);

        triangle_t clipped[MAX_NUM_POLY_TRIANGLES];
        int num_clipped = 0;
        triangles_from_polygon(&poly, clipped, &num_clipped);

        for (int t = 0; t < num_clipped; t++) {
            if (*num_out >= MAX_SKYBOX_TRIANGLES) break;

            triangle_t* ct = &clipped[t];
            vec4_t pts[3];

            for (int j = 0; j < 3; j++) {
                pts[j] = mat4_mul_vec4_project(proj_matrix, ct->points[j]);
                pts[j].y *= -1.0f;
                pts[j].x *= (float)get_window_width()  / 2.0f;
                pts[j].y *= (float)get_window_height() / 2.0f;
                pts[j].x += (float)(get_window_width()  / 2);
                pts[j].y += (float)(get_window_height() / 2);
            }

            triangle_t out;
            out.points[0] = pts[0];
            out.points[1] = pts[1];
            out.points[2] = pts[2];
            out.tex_coords[0] = ct->tex_coords[0];
            out.tex_coords[1] = ct->tex_coords[1];
            out.tex_coords[2] = ct->tex_coords[2];
            out.color   = 0xFFFFFFFF;
            out.texture = face_textures[tri->face_idx];

            out_triangles[(*num_out)++] = out;
        }
    }
}
