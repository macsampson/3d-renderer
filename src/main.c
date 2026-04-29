#include "array.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"
#include "vector.h"
#include "camera.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>

// triangle_t* triangles_to_render = NULL;
#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

// vec3_t camera_pos = {0, 0, 0};
mat4_t proj_matrix;
mat4_t world_matrix;
mat4_t view_matrix;
// vec3_t mesh.rotation = {.x = 0, .y = 0, .z = 0};

// float fov_vector = 640;
bool is_running = false;
int previous_frame_time = 0;

enum cull_method cull_method;
enum render_method render_method;

void setup(void) {

    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    float fov = 3.141592 / 3.0;
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // manually load hard coded texture data from static array
    // mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
    // texture_width = 64;
    // texture_height = 64;

    // load_cube_mesh_data();
    load_obj_file_data("./assets/drone.obj");

    load_png_texture_data("./assets/drone.png");

    // mesh.translation.y -= 100.0;
    // mesh.translation.z += 200.0;
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
                is_running = false;
            }
            if (event.key.keysym.sym == SDLK_1) {
                render_method = RENDER_WIRE_VERTEX;
            }
            if (event.key.keysym.sym == SDLK_2) {
                render_method = RENDER_WIRE;
            }
            if (event.key.keysym.sym == SDLK_3) {
                render_method = RENDER_FILL_TRI;
            }
            if (event.key.keysym.sym == SDLK_4) {
                render_method = RENDER_FILL_TRI_WIRE;
            }
            if (event.key.keysym.sym == SDLK_5) {
                render_method = RENDER_TEXTURED;
            }
            if (event.key.keysym.sym == SDLK_6) {
                render_method = RENDER_TEXTURED_WIRE;
            }
            if (event.key.keysym.sym == SDLK_c) {
                cull_method = CULL_BACKFACE;
            }
            if (event.key.keysym.sym == SDLK_d) {
                cull_method = CULL_NONE;
            }

            break;
        }
    }
}

// vec2_t project(vec3_t point) {
//     vec2_t projected_point = {
//         .x = (fov_vector * point.x) / point.z,
//         .y = (fov_vector * point.y) / point.z
//     };
//     return projected_point;
// }

void update(void) {

    int wait_time = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (wait_time > 0 && wait_time <= FRAME_TARGET_TIME) {
        SDL_Delay(wait_time);
    }

    previous_frame_time = SDL_GetTicks();

    // intialize array of triangles to render
    // triangles_to_render = NULL;
    num_triangles_to_render = 0;

    mesh.rotation.y += 0.01;
    // mesh.rotation.x += 0.02;
    // mesh.rotation.z += 0.01;

    // mesh.scale.x += 0.002;
    // mesh.scale.y += 0.001;
    // mesh.translation.y -= 1.0;
    mesh.translation.z = 5.0;
    camera.position.x += 0.008;
    camera.position.y += 0.008;

    // create teh view matrix looking at a hard coded traget point
    vec3_t target = {0,0,5};
    vec3_t up = {0,1,0};
    view_matrix = mat4_look_at(camera.position, target, up);

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);

    mat4_t translation_matrix =
        mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        // loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // // use matrix to scale our original vertex

            // world matrix
            mat4_t world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // multiply the view matrix by the vector to transform the scene into vector space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            // transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            // transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            // transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            // transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            // transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

            // transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            // transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            // transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // translate vertex away from the camera
            // transformed_vertex.z += 5;

            transformed_vertices[j] = transformed_vertex;
        }

        // backface culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        // find vector between the triangle point and the cam origin
        vec3_t origin = {0,0,0};
        vec3_t camera_ray = vec3_sub(origin, vector_a);

        float dot_normal_cam = vec3_dot(normal, camera_ray);

        if (cull_method == CULL_BACKFACE) {
            if (dot_normal_cam < 0)
                continue;
        }

        vec4_t projected_points[3];

        // loop all vertices to perform the projection
        for (int j = 0; j < 3; j++) {

            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            projected_points[j].y *= -1;

            // scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // translate
            projected_points[j].x += (int)(window_width / 2);
            projected_points[j].y += (int)(window_height / 2);
        }

        float light_intensity_factor = -vec3_dot(normal, light.dir);
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

        triangle_t projected_triangle = {
            .points =
                {
                    {projected_points[0].x,
                     projected_points[0].y,
                     projected_points[0].z,
                     projected_points[0].w},
                    {projected_points[1].x,
                     projected_points[1].y,
                     projected_points[1].z,
                     projected_points[1].w},
                    {projected_points[2].x,
                     projected_points[2].y,
                     projected_points[2].z,
                     projected_points[2].w},
                },
            .tex_coords = {mesh_face.a_uv, mesh_face.b_uv, mesh_face.c_uv},
            .color = triangle_color
        };

        // really bad for performance!!
        // array_push(triangles_to_render, projected_triangle);
        if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
            triangles_to_render[num_triangles_to_render] = projected_triangle;
            num_triangles_to_render++;
        }
    }

    // quicksort
    // int num_triangles = array_length(triangles_to_render);
    // qsort(triangles_to_render, num_triangles, sizeof(triangle_t), compare_triangle_depth);
}

void render(void) {

    draw_grid();

    // Loop all projected points and render
    // int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles_to_render; i++) {

        triangle_t triangle = triangles_to_render[i];

        // draw vertices
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }

        // draw triangles
        if (render_method == RENDER_FILL_TRI || render_method == RENDER_FILL_TRI_WIRE) {
            draw_filled_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.color
            );
        }

        // draw textured triangle
        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE) {
            draw_textured_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.tex_coords[0].u,
                triangle.tex_coords[0].v,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.tex_coords[1].u,
                triangle.tex_coords[1].v,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.tex_coords[2].u,
                triangle.tex_coords[2].v,
                mesh_texture
            );
        }

        if (render_method == RENDER_WIRE || render_method == RENDER_FILL_TRI_WIRE ||
            render_method == RENDER_WIRE_VERTEX || render_method == RENDER_TEXTURED_WIRE) {
            draw_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                0xFF00FF00
            );
        }
    }

    // draw_filled_triangle(300, 200, 50, 400, 500, 700, 0xFF00FF00);

    // array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0x00000000);
    clear_z_buffer();

    SDL_RenderPresent(renderer);
}

void free_resources(void) {
    free(color_buffer);
    free(z_buffer);
    upng_free(png_texture);
    array_free(mesh.faces);
    array_free(mesh.vertices);
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
    free_resources();

    return 0;
}
