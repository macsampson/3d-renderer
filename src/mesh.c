#include "mesh.h"
#include "array.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"
#include "vector.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(
    char* obj_filename,
    char* png_filename,
    vec3_t scale,
    vec3_t translation,
    vec3_t rotation
) {
    load_mesh_obj_data(&meshes[mesh_count], obj_filename);
    load_mesh_png_data(&meshes[mesh_count], png_filename);

    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;

    mesh_count++;
}

static void load_mesh_mtl_data(mesh_t* mesh, char* mtl_filename, const char* obj_dir) {
    FILE* fptr = fopen(mtl_filename, "r");
    if (fptr == NULL) {
        printf("Could not open MTL file: %s\n", mtl_filename);
        return;
    }

    char line[256];
    int current = -1;

    while (fgets(line, sizeof(line), fptr)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (strncmp(p, "newmtl ", 7) == 0) {
            material_t mat = {0};
            sscanf(p + 7, "%63s", mat.name);
            array_push(mesh->materials, mat);
            current = array_length(mesh->materials) - 1;
        } else if (strncmp(p, "map_Kd ", 7) == 0 && current >= 0) {
            char tex_name[256];
            sscanf(p + 7, "%255s", tex_name);

            char tex_path[512];
            snprintf(tex_path, sizeof(tex_path), "%s/%s", obj_dir, tex_name);

            upng_t* png = upng_new_from_file(tex_path);
            if (png != NULL) {
                upng_decode(png);
                if (upng_get_error(png) == UPNG_EOK) {
                    mesh->materials[current].texture = png;
                } else {
                    upng_free(png);
                }
            }
        }
    }
    fclose(fptr);
}

void load_mesh_obj_data(mesh_t* mesh, char* obj_filename) {
    FILE* fptr = fopen(obj_filename, "r");

    if (fptr == NULL) {
        printf("Not able to open the file.");
        return;
    }

    // derive directory of the OBJ so we can resolve relative MTL/texture paths
    char obj_path_copy[512];
    strncpy(obj_path_copy, obj_filename, sizeof(obj_path_copy) - 1);
    obj_path_copy[sizeof(obj_path_copy) - 1] = '\0';
    char* obj_dir = dirname(obj_path_copy);

    char line[256];
    float x, y, z;
    int current_material_index = -1;

    tex2_t* tex_coords = NULL;

    while (fgets(line, sizeof(line), fptr)) {

        // load MTL file referenced by this OBJ
        if (strncmp(line, "mtllib ", 7) == 0) {
            char mtl_name[256];
            sscanf(line + 7, "%255s", mtl_name);
            char mtl_path[512];
            snprintf(mtl_path, sizeof(mtl_path), "%s/%s", obj_dir, mtl_name);
            load_mesh_mtl_data(mesh, mtl_path, obj_dir);
        }

        // track active material for subsequent faces
        if (strncmp(line, "usemtl ", 7) == 0) {
            char mat_name[64];
            sscanf(line + 7, "%63s", mat_name);
            current_material_index = -1;
            int num_mats = array_length(mesh->materials);
            for (int i = 0; i < num_mats; i++) {
                if (strcmp(mesh->materials[i].name, mat_name) == 0) {
                    current_material_index = i;
                    break;
                }
            }
        }

        // read and load vertices from obj
        if (line[0] == 'v' && line[1] == ' ') {
            int matched = sscanf(line, "v %f %f %f", &x, &y, &z);
            if (matched != 3) {
                printf("Malformed obj file.");
                break;
            }
            vec3_t vertex = {.x = x, .y = y, .z = z};
            array_push(mesh->vertices, vertex);
        }

        // read and load texture information
        if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            tex2_t tex_coord;
            sscanf(line, "vt %f %f", &tex_coord.u, &tex_coord.v);
            array_push(tex_coords, tex_coord);
        }

        // read and load faces from obj
        if (line[0] == 'f' && line[1] == ' ') {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];

            int matched = sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0],
                &texture_indices[0],
                &normal_indices[0],
                &vertex_indices[1],
                &texture_indices[1],
                &normal_indices[1],
                &vertex_indices[2],
                &texture_indices[2],
                &normal_indices[2]
            );
            if (matched != 9) {
                sscanf(
                    line,
                    "f %d/%d %d/%d %d/%d",
                    &vertex_indices[0],
                    &texture_indices[0],
                    &vertex_indices[1],
                    &texture_indices[1],
                    &vertex_indices[2],
                    &texture_indices[2]
                );
            }

            face_t face = {
                .a = vertex_indices[0] - 1,
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,
                .a_uv = tex_coords[texture_indices[0] - 1],
                .b_uv = tex_coords[texture_indices[1] - 1],
                .c_uv = tex_coords[texture_indices[2] - 1],
                .color = 0xFFFFFFFF,
                .material_index = current_material_index,
            };

            array_push(mesh->faces, face);
        }
    }
    array_free(tex_coords);
    fclose(fptr);
}

void load_mesh_png_data(mesh_t* mesh, char* filename) {
    upng_t* png_image = upng_new_from_file(filename);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) == UPNG_EOK) {
            mesh->texture = png_image;
        }
    }
    // upng_free(png_texture);
}

int get_num_meshes() { return mesh_count; }

mesh_t* get_mesh(int index) { return &meshes[index]; }

void free_meshes() {
    for (int i = 0; i < mesh_count; i++) {
        upng_free(meshes[i].texture);
        int num_mats = array_length(meshes[i].materials);
        for (int j = 0; j < num_mats; j++) {
            upng_free(meshes[i].materials[j].texture);
        }
        array_free(meshes[i].materials);
        array_free(meshes[i].faces);
        array_free(meshes[i].vertices);
    }
}