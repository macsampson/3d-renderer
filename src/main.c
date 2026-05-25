#include "array.h"
#include "camera.h"
#include "clipping.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"
#include "vector.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <math.h>
#include <stdint.h>

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
float delta_time;

// enum cull_method cull_method;
// enum render_method render_method;

void setup(void) {

    set_render_method(RENDER_WIRE);
    set_cull_method(CULL_BACKFACE);

    // initialize light
    init_light(vec3_new(0, 0, 1));

    // initialize camera
    init_camera(vec3_new(0, 0, 0), vec3_new(0, 0, 1));

    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float aspect_y = (float)get_window_height() / (float)get_window_width();
    float fov_y = 3.141592 / 3.0;
    float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2.0;
    float z_near = 0.1;
    float z_far = 100.0;
    proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

    // initialize frustum planes with a point and a normal
    init_frustum_planes(fov_x, fov_y, z_near, z_far);

    // TODO
    load_mesh(
        "./assets/f22.obj",
        "./assets/f22.png",
        vec3_new(1, 1, 1),
        vec3_new(-3, 0, 8),
        vec3_new(0, 0, 0)
    );
    load_mesh(
        "./assets/efa.obj",
        "./assets/efa.png",
        vec3_new(1, 1, 1),
        vec3_new(3, 0, 8),
        vec3_new(0, 0, 0)
    );
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
                break;
            }
            if (event.key.keysym.sym == SDLK_1) {
                set_render_method(RENDER_WIRE_VERTEX);
                break;
            }
            if (event.key.keysym.sym == SDLK_2) {
                set_render_method(RENDER_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_3) {
                set_render_method(RENDER_FILL_TRI);
                break;
            }
            if (event.key.keysym.sym == SDLK_4) {
                set_render_method(RENDER_FILL_TRI_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_5) {
                set_render_method(RENDER_TEXTURED);
                break;
            }
            if (event.key.keysym.sym == SDLK_6) {
                set_render_method(RENDER_TEXTURED_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_c) {
                set_cull_method(CULL_BACKFACE);
                break;
            }
            if (event.key.keysym.sym == SDLK_x) {
                set_cull_method(CULL_NONE);
                break;
            }
            if (event.key.keysym.sym == SDLK_e) {
                // camera.position.y += 3.0 * delta_time;
                // vec3_t pos = get_camera_position();
                // set_camera_position(vec3_t position);
                break;
            }
            if (event.key.keysym.sym == SDLK_q) {
                // camera.position.y -= 3.0 * delta_time;
                break;
            }
            if (event.key.keysym.sym == SDLK_a) {
                // camera.yaw_angle -= 1.0 * delta_time;
                rotate_camera_yaw(-1.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_d) {
                // camera.yaw_angle += 1.0 * delta_time;
                rotate_camera_yaw(1.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_w) {
                set_camera_forward_velocity(vec3_mult(get_camera_direction(), 5.0 * delta_time));
                set_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                break;
            }
            if (event.key.keysym.sym == SDLK_s) {
                set_camera_forward_velocity(vec3_mult(get_camera_direction(), 5.0 * delta_time));
                set_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                break;
            }
            if (event.key.keysym.sym == SDLK_UP) {
                rotate_camera_pitch(3.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                rotate_camera_pitch(-3.0 * delta_time);
                break;
            }

            break;
        }
    }
}

void process_graphics_pipeline_stages(mesh_t* mesh) {
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix =
        mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

    // offset the camera in the direction im looking at
    vec3_t target = get_camera_lookat_target();
    vec3_t up_dir = vec3_new(0, 1, 0);

    view_matrix = mat4_look_at(get_camera_position(), target, up_dir);

    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++) {

        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

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

            transformed_vertices[j] = transformed_vertex;
        }

        // calculate the triangle face normal
        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        // backface culling testing
        if (is_cull_backface()) {

            // find vector between the triangle point and the cam origin

            vec3_t camera_ray =
                vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

            float dot_normal_cam = vec3_dot(face_normal, camera_ray);

            if (dot_normal_cam < 0)
                continue;
        }

        // create a poly from the original transformed triangle to be clipped
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );

        // clip the polygon and returns a new pilygon with potential new vertices
        clip_polygon(&polygon);

        // break into triangles
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // loop the assembled triangles after clipping
        for (int t = 0; t < num_triangles_after_clipping; t++) {

            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            vec4_t projected_points[3];

            // loop all vertices to perform the projection
            for (int j = 0; j < 3; j++) {

                projected_points[j] =
                    mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                projected_points[j].y *= -1;

                // scale into the view
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // translate
                projected_points[j].x += (int)(get_window_width() / 2);
                projected_points[j].y += (int)(get_window_height() / 2);
            }

            float light_intensity_factor = -vec3_dot(face_normal, get_light_dir());
            uint32_t triangle_color =
                light_apply_intensity(mesh_face.color, light_intensity_factor);

            // create final triangle that will be rendered in screen space
            triangle_t triangle_to_render = {
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
                .tex_coords =
                    {{triangle_after_clipping.tex_coords[0].u,
                      triangle_after_clipping.tex_coords[0].v},
                     {triangle_after_clipping.tex_coords[1].u,
                      triangle_after_clipping.tex_coords[1].v},
                     {triangle_after_clipping.tex_coords[2].u,
                      triangle_after_clipping.tex_coords[2].v}},
                .color = triangle_color,
                .texture = mesh->texture
            };

            // really bad for performance!!
            // array_push(triangles_to_render, projected_triangle);
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
        }
    }
}

void update(void) {

    int wait_time = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (wait_time > 0 && wait_time <= FRAME_TARGET_TIME) {
        SDL_Delay(wait_time);
    }

    // get a delta time fator
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    previous_frame_time = SDL_GetTicks();

    // intialize array of triangles to render
    // triangles_to_render = NULL;
    num_triangles_to_render = 0;

    for (int mesh_idx = 0; mesh_idx < get_num_meshes(); mesh_idx++) {

        mesh_t* mesh = get_mesh(mesh_idx);

        // mesh.rotation.y += 0.6 * delta_time;
        // mesh.rotation.x += 0.6 * delta_time ;
        // mesh.rotation.z += 0.6 * delta_time;

        // mesh.scale.x += 0.002;
        // mesh.scale.y += 0.001;
        // mesh.translation.y -= 1.0;
        // mesh.translation.z = 5.0;

        // process mesh
        process_graphics_pipeline_stages(mesh);
    }
}

// quicksort
// int num_triangles = array_length(triangles_to_render);
// qsort(triangles_to_render, num_triangles, sizeof(triangle_t), compare_triangle_depth);

void render(void) {
    clear_color_buffer(0x00000000);
    clear_z_buffer();

    draw_grid();

    // Loop all projected points and render
    // int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles_to_render; i++) {

        triangle_t triangle = triangles_to_render[i];

        // draw vertices
        if (should_render_wire_vertex()) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }

        // draw triangles
        if (should_render_filled_triangles()) {
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
        if (should_render_textured_triangles()) {
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
                triangle.texture
            );
        }

        if (should_render_wireframe()) {
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
}

void free_resources(void) {
    free_meshes();
    destroy_window();
}

int main(void) {

    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    free_resources();

    return 0;
}
