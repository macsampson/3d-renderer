#pragma once
#define MESH_H

#include "matrix.h"
#include "triangle.h"
#include "upng.h"
#include "vector.h"
#include <stdbool.h>

typedef struct {
    char name[64];
    upng_t* texture;
} material_t;

typedef struct {
    vec3_t* vertices;
    face_t* faces;
    upng_t* texture;
    material_t* materials;
    vec3_t rotation;
    vec3_t scale;
    vec3_t translation;
    mat4_t cached_rot_x, cached_rot_y, cached_rot_z;
    vec3_t cached_rotation;
    bool rotation_cache_valid;
} mesh_t;

void load_mesh(
    char* obj_filename,
    char* png_filename,
    vec3_t scale,
    vec3_t translation,
    vec3_t rotation
);
void load_mesh_obj_data(mesh_t* mesh, char* obj_filename);
void load_mesh_png_data(mesh_t* mesh, char* filename);

int get_num_meshes(void);
mesh_t* get_mesh(int index);

void free_meshes(void);