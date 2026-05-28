#include "triangle.h"
#include "display.h"
#include "swap.h"
#include "texture.h"
#include "upng.h"
#include "vector.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

vec3_t get_triangle_normal(vec4_t vertices[3]) {
    // backface culling
    vec3_t vector_a = vec3_from_vec4(vertices[0]);
    vec3_t vector_b = vec3_from_vec4(vertices[1]);
    vec3_t vector_c = vec3_from_vec4(vertices[2]);

    vec3_t vector_ab = vec3_sub(vector_b, vector_a);
    vec3_t vector_ac = vec3_sub(vector_c, vector_a);
    vec3_normalize(&vector_ab);
    vec3_normalize(&vector_ac);

    vec3_t normal = vec3_cross(vector_ab, vector_ac);
    vec3_normalize(&normal);

    return normal;
}

void draw_filled_triangle(
    int x0, int y0, float w0,
    int x1, int y1, float w1,
    int x2, int y2, float w2,
    uint32_t color
) {
    if (y0 > y1) { int_swap(&x0, &x1); int_swap(&y0, &y1); float_swap(&w0, &w1); }
    if (y1 > y2) { int_swap(&x1, &x2); int_swap(&y1, &y2); float_swap(&w1, &w2); }
    if (y0 > y1) { int_swap(&x0, &x1); int_swap(&y0, &y1); float_swap(&w0, &w1); }

    // 2.1+1.2: incremental interpolation with per-vertex reciprocals
    int win_w = get_window_width();
    int win_h = get_window_height();
    uint32_t* cbuf = get_color_buffer();
    float* zbuf = get_z_buffer();

    float inv_w0 = 1.0f / w0, inv_w1 = 1.0f / w1, inv_w2 = 1.0f / w2;

    int dy01 = y1 - y0, dy02 = y2 - y0, dy12 = y2 - y1;

    float dx_01 = dy01 ? (float)(x1 - x0) / dy01 : 0.0f;
    float dx_02 = dy02 ? (float)(x2 - x0) / dy02 : 0.0f;
    float dx_12 = dy12 ? (float)(x2 - x1) / dy12 : 0.0f;

    float diw_01 = dy01 ? (inv_w1 - inv_w0) / dy01 : 0.0f;
    float diw_02 = dy02 ? (inv_w2 - inv_w0) / dy02 : 0.0f;
    float diw_12 = dy12 ? (inv_w2 - inv_w1) / dy12 : 0.0f;

    // upper half: y0..y1
    if (dy01) {
        int ys = y0 < 0 ? 0 : y0;
        int ye = y1 > win_h - 1 ? win_h - 1 : y1;
        for (int y = ys; y <= ye; y++) {
            float t = (float)(y - y0);
            float xa = x0 + t * dx_01;
            float xb = x0 + t * dx_02;
            float iwa = inv_w0 + t * diw_01;
            float iwb = inv_w0 + t * diw_02;
            if (xa > xb) { float tmp; tmp=xa; xa=xb; xb=tmp; tmp=iwa; iwa=iwb; iwb=tmp; }
            // 2.2: clamp x to viewport
            int xs = (int)xa; xs = xs < 0 ? 0 : xs;
            int xe = (int)xb; xe = xe > win_w ? win_w : xe;
            float span = xb - xa;
            float inv_span = span > 0.5f ? 1.0f / span : 0.0f;
            float d_iw = (iwb - iwa) * inv_span;
            float iw = iwa + (xs - xa) * d_iw;
            int row = y * win_w;
            for (int x = xs; x < xe; x++) {
                float depth = 1.0f - iw;
                if (depth < zbuf[row + x]) { cbuf[row + x] = color; zbuf[row + x] = depth; }
                iw += d_iw;
            }
        }
    }

    // lower half: y1..y2
    if (dy12) {
        int ys = y1 < 0 ? 0 : y1;
        int ye = y2 > win_h - 1 ? win_h - 1 : y2;
        for (int y = ys; y <= ye; y++) {
            float t12 = (float)(y - y1);
            float t02 = (float)(y - y0);
            float xa = x1 + t12 * dx_12;
            float xb = x0 + t02 * dx_02;
            float iwa = inv_w1 + t12 * diw_12;
            float iwb = inv_w0 + t02 * diw_02;
            if (xa > xb) { float tmp; tmp=xa; xa=xb; xb=tmp; tmp=iwa; iwa=iwb; iwb=tmp; }
            int xs = (int)xa; xs = xs < 0 ? 0 : xs;
            int xe = (int)xb; xe = xe > win_w ? win_w : xe;
            float span = xb - xa;
            float inv_span = span > 0.5f ? 1.0f / span : 0.0f;
            float d_iw = (iwb - iwa) * inv_span;
            float iw = iwa + (xs - xa) * d_iw;
            int row = y * win_w;
            for (int x = xs; x < xe; x++) {
                float depth = 1.0f - iw;
                if (depth < zbuf[row + x]) { cbuf[row + x] = color; zbuf[row + x] = depth; }
                iw += d_iw;
            }
        }
    }
};

void draw_textured_triangle(
    int x0, int y0, float w0, float u0, float v0,
    int x1, int y1, float w1, float u1, float v1,
    int x2, int y2, float w2, float u2, float v2,
    upng_t* texture
) {
    if (y0 > y1) { int_swap(&x0,&x1); int_swap(&y0,&y1); float_swap(&w0,&w1); float_swap(&u0,&u1); float_swap(&v0,&v1); }
    if (y1 > y2) { int_swap(&x1,&x2); int_swap(&y1,&y2); float_swap(&w1,&w2); float_swap(&u1,&u2); float_swap(&v1,&v2); }
    if (y0 > y1) { int_swap(&x0,&x1); int_swap(&y0,&y1); float_swap(&w0,&w1); float_swap(&u0,&u1); float_swap(&v0,&v1); }

    // flip v after sort (keeps v in [0,1], equivalent to fabsf(v-1))
    v0 = 1.0f - v0; v1 = 1.0f - v1; v2 = 1.0f - v2;

    // 1.5: hoist depth bypass out of scanline loop
    bool bypass = is_depth_bypass();

    // 2.5: hoist texture dimensions; get buffer pointers once
    int tex_w = upng_get_width(texture);
    int tex_h = upng_get_height(texture);
    uint32_t* tex_buf = (uint32_t*)upng_get_buffer(texture);

    // 2.2: hoist window bounds and direct buffer access
    int win_w = get_window_width();
    int win_h = get_window_height();
    uint32_t* cbuf = get_color_buffer();
    float* zbuf = get_z_buffer();

    // 1.2: per-vertex reciprocals computed once per triangle
    float inv_w0 = 1.0f / w0, inv_w1 = 1.0f / w1, inv_w2 = 1.0f / w2;

    // 1.2+2.1: UV/w for perspective-correct interpolation
    float u0w = u0 * inv_w0, u1w = u1 * inv_w1, u2w = u2 * inv_w2;
    float v0w = v0 * inv_w0, v1w = v1 * inv_w1, v2w = v2 * inv_w2;

    int dy01 = y1 - y0, dy02 = y2 - y0, dy12 = y2 - y1;

    float dx_01 = dy01 ? (float)(x1 - x0) / dy01 : 0.0f;
    float dx_02 = dy02 ? (float)(x2 - x0) / dy02 : 0.0f;
    float dx_12 = dy12 ? (float)(x2 - x1) / dy12 : 0.0f;

    float diw_01 = dy01 ? (inv_w1 - inv_w0) / dy01 : 0.0f;
    float diw_02 = dy02 ? (inv_w2 - inv_w0) / dy02 : 0.0f;
    float diw_12 = dy12 ? (inv_w2 - inv_w1) / dy12 : 0.0f;

    float duw_01 = dy01 ? (u1w - u0w) / dy01 : 0.0f;
    float duw_02 = dy02 ? (u2w - u0w) / dy02 : 0.0f;
    float duw_12 = dy12 ? (u2w - u1w) / dy12 : 0.0f;

    float dvw_01 = dy01 ? (v1w - v0w) / dy01 : 0.0f;
    float dvw_02 = dy02 ? (v2w - v0w) / dy02 : 0.0f;
    float dvw_12 = dy12 ? (v2w - v1w) / dy12 : 0.0f;

    // upper half: y0..y1, edges 0->1 and 0->2
    if (dy01) {
        int ys = y0 < 0 ? 0 : y0;
        int ye = y1 > win_h - 1 ? win_h - 1 : y1;
        for (int y = ys; y <= ye; y++) {
            float t = (float)(y - y0);
            float xa = x0 + t * dx_01,  xb = x0 + t * dx_02;
            float iwa = inv_w0 + t * diw_01, iwb = inv_w0 + t * diw_02;
            float uwa = u0w + t * duw_01,    uwb = u0w + t * duw_02;
            float vwa = v0w + t * dvw_01,    vwb = v0w + t * dvw_02;
            if (xa > xb) {
                float tmp;
                tmp=xa; xa=xb; xb=tmp; tmp=iwa; iwa=iwb; iwb=tmp;
                tmp=uwa; uwa=uwb; uwb=tmp; tmp=vwa; vwa=vwb; vwb=tmp;
            }
            // 2.2: clamp x to viewport
            int xs = (int)xa; xs = xs < 0 ? 0 : xs;
            int xe = bypass ? (int)xb + 1 : (int)xb;
            xe = xe > win_w ? win_w : xe;
            float span = xb - xa;
            float inv_span = span > 0.5f ? 1.0f / span : 0.0f;
            float d_iw = (iwb - iwa) * inv_span;
            float d_uw = (uwb - uwa) * inv_span;
            float d_vw = (vwb - vwa) * inv_span;
            float off = xs - xa;
            float iw = iwa + off * d_iw;
            float uw = uwa + off * d_uw;
            float vw = vwa + off * d_vw;
            int row = y * win_w;
            for (int x = xs; x < xe; x++) {
                float recip = 1.0f / iw;
                // 2.4+2.5: branchless UV clamp, pre-scaled by texture dims
                int tx = (int)(uw * recip * tex_w);
                int ty = (int)(vw * recip * tex_h);
                tx = tx < 0 ? 0 : (tx >= tex_w ? tex_w - 1 : tx);
                ty = ty < 0 ? 0 : (ty >= tex_h ? tex_h - 1 : ty);
                uint32_t texel = tex_buf[tex_w * ty + tx];
                if (bypass) {
                    cbuf[row + x] = texel;
                } else {
                    float depth = 1.0f - iw;
                    if (depth < zbuf[row + x]) { cbuf[row + x] = texel; zbuf[row + x] = depth; }
                }
                iw += d_iw; uw += d_uw; vw += d_vw;
            }
        }
    }

    // lower half: y1..y2, edges 1->2 and 0->2
    if (dy12) {
        int ys = y1 < 0 ? 0 : y1;
        int ye = y2 > win_h - 1 ? win_h - 1 : y2;
        for (int y = ys; y <= ye; y++) {
            float t12 = (float)(y - y1);
            float t02 = (float)(y - y0);
            float xa = x1 + t12 * dx_12,  xb = x0 + t02 * dx_02;
            float iwa = inv_w1 + t12 * diw_12, iwb = inv_w0 + t02 * diw_02;
            float uwa = u1w + t12 * duw_12,    uwb = u0w + t02 * duw_02;
            float vwa = v1w + t12 * dvw_12,    vwb = v0w + t02 * dvw_02;
            if (xa > xb) {
                float tmp;
                tmp=xa; xa=xb; xb=tmp; tmp=iwa; iwa=iwb; iwb=tmp;
                tmp=uwa; uwa=uwb; uwb=tmp; tmp=vwa; vwa=vwb; vwb=tmp;
            }
            int xs = (int)xa; xs = xs < 0 ? 0 : xs;
            int xe = bypass ? (int)xb + 1 : (int)xb;
            xe = xe > win_w ? win_w : xe;
            float span = xb - xa;
            float inv_span = span > 0.5f ? 1.0f / span : 0.0f;
            float d_iw = (iwb - iwa) * inv_span;
            float d_uw = (uwb - uwa) * inv_span;
            float d_vw = (vwb - vwa) * inv_span;
            float off = xs - xa;
            float iw = iwa + off * d_iw;
            float uw = uwa + off * d_uw;
            float vw = vwa + off * d_vw;
            int row = y * win_w;
            for (int x = xs; x < xe; x++) {
                float recip = 1.0f / iw;
                int tx = (int)(uw * recip * tex_w);
                int ty = (int)(vw * recip * tex_h);
                tx = tx < 0 ? 0 : (tx >= tex_w ? tex_w - 1 : tx);
                ty = ty < 0 ? 0 : (ty >= tex_h ? tex_h - 1 : ty);
                uint32_t texel = tex_buf[tex_w * ty + tx];
                if (bypass) {
                    cbuf[row + x] = texel;
                } else {
                    float depth = 1.0f - iw;
                    if (depth < zbuf[row + x]) { cbuf[row + x] = texel; zbuf[row + x] = depth; }
                }
                iw += d_iw; uw += d_uw; vw += d_vw;
            }
        }
    }
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // Find the vectors between the vertices ABC and point p
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);

    // Compute the area of the full parallegram/triangle ABC using 2D cross product
    float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||

    // Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full
    // parallelogram/triangle ABC
    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

    // Beta is the area of the small parallelogram/triangle APC divided by the area of the full
    // parallelogram/triangle ABC
    float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;

    // Weight gamma is easily found since barycentric coordinates always add up to 1.0
    float gamma = 1 - alpha - beta;

    vec3_t weights = {alpha, beta, gamma};
    return weights;
}

void draw_triangle_pixel(
    int x,
    int y,
    uint32_t color,
    vec4_t point_a,
    vec4_t point_b,
    vec4_t point_c
) {
    vec2_t p = {x, y};
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // used for perspective correction TODO: optimize this by moving division out of funciton
    float interpolated_reciprocal_w =
        (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // adjust 1/w so the pixels that are closer to the viwer have smaller values
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

    // only draw the pixel if the depth value is less than the one previously stored in the z buffer
    if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {

        draw_pixel(x, y, color);

        // update z buffer value with the 1/w with this pixel
        update_zbuffer_at(x, y, interpolated_reciprocal_w);
    }
}

void draw_texel(
    int x,
    int y,
    upng_t* texture,
    vec4_t point_a,
    vec4_t point_b,
    vec4_t point_c,
    tex2_t a_uv,
    tex2_t b_uv,
    tex2_t c_uv
) {
    vec2_t p = {x, y};
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, p);
    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // perform interpolation of all U and V values using barycentric weights
    interpolated_u =
        (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
    interpolated_v =
        (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

    // used for perspective correction TODO: optimize this by moving division out of funciton
    interpolated_reciprocal_w =
        (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // get mesh texture widht and height dimensions
    int texture_width = upng_get_width(texture);
    int texture_height = upng_get_height(texture);

    // map the uv coord to the full texture width and height
    int tex_x = (int)(fabsf(interpolated_u) * texture_width);
    int tex_y = (int)(fabsf(interpolated_v) * texture_height);
    if (tex_x < 0 || tex_x >= texture_width)  tex_x = 0;
    if (tex_y < 0 || tex_y >= texture_height) tex_y = 0;

    uint32_t* texture_buffer = (uint32_t*)upng_get_buffer(texture);
    uint32_t texel = texture_buffer[(texture_width * tex_y) + tex_x];

    if (is_depth_bypass()) {
        draw_pixel(x, y, texel);
    } else {
        // adjust 1/w so the pixels that are closer to the viewer have smaller values
        interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;
        if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {
            draw_pixel(x, y, texel);
            update_zbuffer_at(x, y, interpolated_reciprocal_w);
        }
    }
}