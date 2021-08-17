#ifndef GX_INCLUDED
/*
    gx.h -- simple sprite library wrapper for sokol gfx

    OVERVIEW
    ========

    todo...

    PROVIDED FUNCTIONS
    ==================

    todo...

    LICENSE
    =======
    zlib/libpng license

    Copyright (c) 2021 Wade Brainerd

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define GX_INCLUDED (1)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width, height;
    sg_image image;
} gx_sprite;

void gx_setup(void);
void gx_shutdown(void);

void gx_begin_drawing();
void gx_end_drawing();

gx_sprite gx_make_sprite(const char *path);
void gx_draw_sprite(int x, int y, gx_sprite *sprite);

void gx_draw_rect(int x, int y, int width, int height, sg_color color);

#ifdef __cplusplus
}
#endif

#endif // GX_INCLUDED

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef GX_IMPL
#define GX_IMPL_INCLUDED (1)

#include "gx.glsl.h"

static struct {
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
    sg_image white_image;
    sg_color white_color;
} gx_state;

typedef struct {
    float x, y, z;
    uint32_t color;
    int16_t u, v;
} gx_vertex_t;

void gx_setup(void) {
    gx_vertex_t vertices[] = {
        /* pos                  color       uvs */
        {  0.0f,  0.0f,  1.0f,  0xFFFFFFFF,     0,     0 },
        {  1.0f,  0.0f,  1.0f,  0xFFFFFFFF, 32767,     0 },
        {  1.0f,  1.0f,  1.0f,  0xFFFFFFFF, 32767, 32767 },
        {  0.0f,  1.0f,  1.0f,  0xFFFFFFFF,     0, 32767 },
    };

    gx_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });

    uint16_t indices[] = {
        2, 1, 0,  3, 2, 0,
    };
    gx_state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "quad-indices"
    });

    uint32_t white_pixels[4*4] = {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    };
    gx_state.white_image = sg_make_image(&(sg_image_desc){
        .width = 4,
        .height = 4,
        .data.subimage[0][0] = SG_RANGE(white_pixels),
        .label = "white_image"
    });

    gx_state.white_color = (sg_color){ 1.0f, 1.0f, 1.0f, 1.0f };

    sg_shader shd = sg_make_shader(texcube_shader_desc(sg_query_backend()));

    gx_state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .attrs = {
                [ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_UBYTE4N,
                [ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N
            }
        },
        .colors[0] = {
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .label = "pip"
    });

    gx_state.pass_action = (sg_pass_action) {
        .colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.0f, 0.0f, 0.0f, 1.0f } }
    };
}

void gx_shutdown() {
}

void gx_begin_drawing() {
    sg_begin_default_pass(&gx_state.pass_action, sapp_width(), sapp_height());
}

void gx_end_drawing() {
    sg_end_pass();
    sg_commit();
}

gx_sprite gx_make_sprite(const char *path) {
    gx_sprite sprite = {
        .width = 0,
        .height = 0,
    };

    int n;
    unsigned char *pixels = stbi_load(path, &sprite.width, &sprite.height, &n, 4);
    if (pixels == NULL) {
        return sprite;
    }

    sprite.image = sg_make_image(&(sg_image_desc){
        .width = sprite.width,
        .height = sprite.height,
        .data.subimage[0][0] = (sg_range){ pixels, sprite.width*sprite.height*4 },
        .label = path
    });

    return sprite;
}

static vs_params_t gx_vs_params(int x, int y, int width, int height, sg_color color) {
    vs_params_t vs_params;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            vs_params.mvp[4*i+j] = i==j? 1.0f : 0.0f;
    vs_params.mvp[0] = width * 2.0f / sapp_width();
    vs_params.mvp[5] = height * -2.0f / sapp_height();
    vs_params.mvp[12] = x * 2.0f / sapp_width() - 1.0f;
    vs_params.mvp[13] = y * -2.0f / sapp_height() + 1.0f;
    vs_params.uniform_color[0] = color.r;
    vs_params.uniform_color[1] = color.g;
    vs_params.uniform_color[2] = color.b;
    vs_params.uniform_color[3] = color.a;
    return vs_params;
}

void gx_draw_sprite(int x, int y, gx_sprite *sprite) {
    sg_apply_pipeline(gx_state.pip);

    gx_state.bind.fs_images[SLOT_tex] = sprite->image;
    sg_apply_bindings(&gx_state.bind);

    vs_params_t vs_params = gx_vs_params(x, y, sprite->width, sprite->height, gx_state.white_color);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));

    sg_draw(0, 6, 1);
}

void gx_draw_rect(int x, int y, int width, int height, sg_color color) {
    sg_apply_pipeline(gx_state.pip);

    gx_state.bind.fs_images[SLOT_tex] = gx_state.white_image;
    sg_apply_bindings(&gx_state.bind);

    vs_params_t vs_params = gx_vs_params(x, y, width, height, color);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));

    sg_draw(0, 6, 1);
}

#endif // GX_IMPL
