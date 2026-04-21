#include "mesh.h"
#include "array.h"
#include "triangle.h"
#include "vector.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

mesh_t mesh = {.vertices = NULL, .faces = NULL, .rotation = {0, 0, 0}};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, // 1
    {.x = -1, .y = 1, .z = -1},  // 2
    {.x = 1, .y = 1, .z = -1},   // 3
    {.x = 1, .y = -1, .z = -1},  // 4
    {.x = 1, .y = 1, .z = 1},    // 5
    {.x = 1, .y = -1, .z = 1},   // 6
    {.x = -1, .y = 1, .z = 1},   // 7
    {.x = -1, .y = -1, .z = 1}   // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    {.a = 1, .b = 2, .c = 3, .color = 0xFFFF0000},
    {.a = 1, .b = 3, .c = 4, .color = 0xFFFF0000},
    // right
    {.a = 4, .b = 3, .c = 5, .color = 0xFF00FF00},
    {.a = 4, .b = 5, .c = 6, .color = 0xFF00FF00},
    // back
    {.a = 6, .b = 5, .c = 7, .color = 0xFF0000FF},
    {.a = 6, .b = 7, .c = 8, .color = 0xFF0000FF},
    // left
    {.a = 8, .b = 7, .c = 2, .color = 0xFFFFFF00},
    {.a = 8, .b = 2, .c = 1, .color = 0xFFFFFF00},
    // top
    {.a = 2, .b = 7, .c = 5, .color = 0xFFFF00FF},
    {.a = 2, .b = 5, .c = 3, .color = 0xFFFF00FF},
    // bottom
    {.a = 6, .b = 8, .c = 1, .color = 0xFF00FFFF},
    {.a = 6, .b = 1, .c = 4, .color = 0xFF00FFFF}
};

void load_cube_mesh_data(void) {
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        array_push(mesh.vertices, cube_vertices[i]);
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        array_push(mesh.faces, cube_faces[i]);
    }
}

void load_obj_file_data(char* filename) {
    FILE* fptr = fopen(filename, "r");

    if (fptr == NULL) {
        printf("Not able to open the file.");
        return;
    }

    char line[256];
    float x, y, z;

    while (fgets(line, sizeof(line), fptr)) {

        // read and load vertices from obj
        if (line[0] == 'v' && line[1] == ' ') {
            int matched = sscanf(line, "v %f %f %f", &x, &y, &z);
            if (matched != 3) {
                printf("Malformed obj file.");
                break;
            }
            vec3_t vertex = {.x = x, .y = y, .z = z};
            array_push(mesh.vertices, vertex);
        }

        // read and load faces from obj
        if (line[0] == 'f' && line[1] == ' ') {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];

            sscanf(
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

            face_t face = {
                .a = vertex_indices[0],
                .b = vertex_indices[1],
                .c = vertex_indices[2],
            };

            array_push(mesh.faces, face);
        }
    }
    fclose(fptr);
}
