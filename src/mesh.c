#include "mesh.h"
#include "array.h"
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
    {.a = 1, .b = 2, .c = 3},
    {.a = 1, .b = 3, .c = 4},
    // right
    {.a = 4, .b = 3, .c = 5},
    {.a = 4, .b = 5, .c = 6},
    // back
    {.a = 6, .b = 5, .c = 7},
    {.a = 6, .b = 7, .c = 8},
    // left
    {.a = 8, .b = 7, .c = 2},
    {.a = 8, .b = 2, .c = 1},
    // top
    {.a = 2, .b = 7, .c = 5},
    {.a = 2, .b = 5, .c = 3},
    // bottom
    {.a = 6, .b = 8, .c = 1},
    {.a = 6, .b = 1, .c = 4}
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
        if (line[0] == 'v' && line[1] == ' ') {
            int matched = sscanf(line + 2, "%f %f %f", &x, &y, &z);
            if (matched != 3) {
                printf("Malformed obj file.");
                break;
            }
            vec3_t vertex = {.x = x, .y = y, .z = z};
            array_push(mesh.vertices, vertex);
        }

        if (line[0] == 'f' && line[1] == ' ') {
            int vertex_indices[3];
            int i = 0;
            char* tok = strtok(line + 2, " ");
            while (tok != NULL && i < 3) {
                sscanf(tok, "%d", &vertex_indices[i]);
                i++;
                tok = strtok(NULL, " ");
            }

            if (i == 3) {
                face_t face = {
                    .a = vertex_indices[0],
                    .b = vertex_indices[1],
                    .c = vertex_indices[2],
                };
                array_push(mesh.faces, face);
            }
        }
    }

    fclose(fptr);
}