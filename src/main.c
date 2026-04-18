#include "display.h"
#include "mesh.h"
#include "triangle.h"
#include "vector.h"
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>

// #define N_POINTS (9 * 9 * 9)
// vec3_t cube_points[N_POINTS];
// vec2_t projected_points[N_POINTS];
triangle_t triangles_to_render[N_MESH_FACES];

vec3_t camera_pos = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

float fov_vector = 640;
bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    // int point_count = 0;

    // for (float x = -1; x <= 1; x += 0.25) {
    //     for (float y = -1; y <= 1; y += 0.25) {
    //         for (float z = -1; z <= 1; z += 0.25) {
    //             vec3_t new_point = {.x = x, .y = y, .z = z};
    //             cube_points[point_count++] = new_point;
    //         }
    //     }
    // }
}

void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                printf("escaped");
                is_running = false;
            }
            break;
        }
    }
}

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_vector * point.x) / point.z,
        .y = (fov_vector * point.y) / point.z
    };
    return projected_point;
}

void update(void) {

    int wait_time = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (wait_time > 0 && wait_time <= FRAME_TARGET_TIME) {
        SDL_Delay(wait_time);
    }

    previous_frame_time = SDL_GetTicks();

    cube_rotation.y += 0.01;
    cube_rotation.x += 0.01;
    cube_rotation.z += 0.01;

    for (int i = 0; i < N_MESH_FACES; i++) {
        face_t mesh_face = mesh_faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh_vertices[mesh_face.a - 1];
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        triangle_t projected_triangle;

        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];
            transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z);

            // translate vertex away from the camera
            transformed_vertex.z -= camera_pos.z;

            vec2_t projected_point = project(transformed_vertex);

            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        triangles_to_render[i] = projected_triangle;
    }
}

void render(void) {
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // SDL_RenderClear(renderer);

    draw_grid();
    // draw_rect(100, 100, 200, 200, 0XFF00FFFF);

    // Loop all projected points and render (also translates)
    for (int i = 0; i < N_MESH_FACES; i++) {
        triangle_t triangle = triangles_to_render[i];
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);
    }

    render_color_buffer();

    clear_color_buffer(0x00000000);

    SDL_RenderPresent(renderer);
}

int main(void) {

    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
