#include "display.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];

vec2_t projected_points[N_POINTS];

float fov_vector = 640;
vec3_t camera_pos = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

bool is_running = false;

void setup(void) {
    color_buffer =
        (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    int point_count = 0;

    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = {.x = x, .y = y, .z = z};
                cube_points[point_count++] = new_point;
            }
        }
    }
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

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

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_vector * point.x) / point.z,
        .y = (fov_vector * point.y) / point.z
    };
    return projected_point;
}

void update(void) {

    cube_rotation.y += 0.01;
    cube_rotation.x += 0.01;
    cube_rotation.z += 0.01;

    for (int i = 0; i < N_POINTS; i++) {
        vec3_t point = cube_points[i];

        vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
        transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
        transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

        // move points away from the camera
        transformed_point.z -= camera_pos.z;

        // Project the current point
        vec2_t projected_point = project(transformed_point);

        // Save the projected 2D vector in the array of projected point
        projected_points[i] = projected_point;
    }
}

void render(void) {
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // SDL_RenderClear(renderer);

    draw_grid();
    // draw_rect(100, 100, 200, 200, 0XFF00FFFF);

    // Loop all projected points and render (also translates)
    for (int i = 0; i < N_POINTS; i++) {
        vec2_t projected_point = projected_points[i];
        draw_rect(
            projected_point.x + (window_width / 2),
            projected_point.y + (window_height / 2),
            4,
            4,
            0xFF00FFFF
        );
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
