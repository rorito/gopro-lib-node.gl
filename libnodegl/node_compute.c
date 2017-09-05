/*
 * Copyright 2017 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "glincludes.h"
#include "log.h"
#include "math_utils.h"
#include "nodegl.h"
#include "nodes.h"
#include "utils.h"

#define TEXTURE_TYPES_LIST    (const int[]){NGL_NODE_TEXTURE,        \
                                            -1}

#define CS_TYPES_LIST         (const int[]){NGL_NODE_COMPUTESHADER,  \
                                            -1}

#define UNIFORMS_TYPES_LIST   (const int[]){NGL_NODE_UNIFORMSCALAR,  \
                                            NGL_NODE_UNIFORMVEC2,    \
                                            NGL_NODE_UNIFORMVEC3,    \
                                            NGL_NODE_UNIFORMVEC4,    \
                                            NGL_NODE_UNIFORMINT,     \
                                            NGL_NODE_UNIFORMMAT4,    \
                                            NGL_NODE_UNIFORMSAMPLER, \
                                            -1}

#define BUFFER_TYPES_LIST (const int[]) {NGL_NODE_BUFFERFLOAT,       \
                                         NGL_NODE_BUFFERVEC2,        \
                                         NGL_NODE_BUFFERVEC3,        \
                                         NGL_NODE_BUFFERVEC4,        \
                                         -1}

#define OFFSET(x) offsetof(struct compute, x)
static const struct node_param compute_params[] = {
    {"nb_group_x", PARAM_TYPE_INT,      OFFSET(nb_group_x), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"nb_group_y", PARAM_TYPE_INT,      OFFSET(nb_group_y), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"nb_group_z", PARAM_TYPE_INT,      OFFSET(nb_group_z), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"shader",     PARAM_TYPE_NODE,     OFFSET(shader),     .flags=PARAM_FLAG_CONSTRUCTOR, .node_types=CS_TYPES_LIST},
    {"textures",   PARAM_TYPE_NODELIST, OFFSET(textures),   .node_types=TEXTURE_TYPES_LIST},
    {"uniforms",   PARAM_TYPE_NODELIST, OFFSET(uniforms),   .node_types=UNIFORMS_TYPES_LIST},
    {"buffers",    PARAM_TYPE_NODELIST, OFFSET(buffers),    .node_types=BUFFER_TYPES_LIST},
    {NULL}
};

static int update_uniforms(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct compute *s = node->priv_data;

    for (int i = 0; i < s->nb_uniforms; i++) {
        const struct ngl_node *unode = s->uniforms[i];
        const struct uniform *u = unode->priv_data;
        const GLint uid = s->uniform_ids[i];
        switch (unode->class->id) {
        case NGL_NODE_UNIFORMSCALAR: ngli_glUniform1f (gl, uid,    u->scalar);                 break;
        case NGL_NODE_UNIFORMVEC2:   ngli_glUniform2fv(gl, uid, 1, u->vector);                 break;
        case NGL_NODE_UNIFORMVEC3:   ngli_glUniform3fv(gl, uid, 1, u->vector);                 break;
        case NGL_NODE_UNIFORMVEC4:   ngli_glUniform4fv(gl, uid, 1, u->vector);                 break;
        case NGL_NODE_UNIFORMINT:    ngli_glUniform1i (gl, uid,    u->ival);                   break;
        case NGL_NODE_UNIFORMMAT4:   ngli_glUniformMatrix4fv(gl, uid, 1, GL_FALSE, u->matrix); break;
        case NGL_NODE_UNIFORMSAMPLER:                                                          break;
        default:
            LOG(ERROR, "unsupported uniform of type %s", unode->class->name);
            break;
        }
    }

    for (int i = 0; i < s->nb_textures; i++) {
        if (!s->textures[i])
            continue;

        struct texture *texture = s->textures[i]->priv_data;
        struct textureshaderinfo *textureshaderinfo = &s->textureshaderinfos[i];

        if (textureshaderinfo->sampler_id >= 0) {
            ngli_glBindImageTexture(gl, textureshaderinfo->sampler_id, texture->id, 0, GL_FALSE, 0, texture->access, texture->internal_format);
        }

        if (textureshaderinfo->dimensions_id >= 0) {
            float dimensions[2] = { texture->width, texture->height };
            ngli_glUniform2fv(gl, textureshaderinfo->dimensions_id, 1, dimensions);
        }
    }


    for (int i = 0; i < s->nb_buffers; i++) {
        struct ngl_node *bnode = s->buffers[i];
        struct buffer *b = bnode->priv_data;

        ngli_glBindBufferBase(gl, GL_SHADER_STORAGE_BUFFER, s->buffer_ids[i], b->buffer_id);
    }

    return 0;
}

static int compute_init(struct ngl_node *node)
{
    int ret;

    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct compute *s = node->priv_data;
    struct computeshader *shader = s->shader->priv_data;

    if (!glcontext->has_cs_compatibility) {
        LOG(ERROR, "Context does not support compute shaders");
        return -1;
    }

    if (s->nb_group_x > glcontext->max_compute_work_group_counts[0] ||
        s->nb_group_y > glcontext->max_compute_work_group_counts[1] ||
        s->nb_group_z > glcontext->max_compute_work_group_counts[2]) {
        LOG(ERROR,
            "Compute work group size (%d, %d, %d) exceeds driver limit (%d, %d, %d)",
            s->nb_group_x,
            s->nb_group_y,
            s->nb_group_z,
            glcontext->max_compute_work_group_counts[0],
            glcontext->max_compute_work_group_counts[1],
            glcontext->max_compute_work_group_counts[2]);
        return -1;
    }

    ret = ngli_node_init(s->shader);
    if (ret < 0)
        return ret;

    if (s->nb_uniforms > 0) {
        s->uniform_ids = calloc(s->nb_uniforms, sizeof(*s->uniform_ids));
        if (!s->uniform_ids)
            return -1;
    }

    for (int i = 0; i < s->nb_uniforms; i++) {
        struct ngl_node *unode = s->uniforms[i];
        struct uniform *u = unode->priv_data;
        ret = ngli_node_init(unode);
        if (ret < 0)
            return ret;
        s->uniform_ids[i] = ngli_glGetUniformLocation(gl, shader->program_id, u->name);
    }

    if (s->nb_buffers > 0) {
        s->buffer_ids = calloc(s->nb_buffers, sizeof(*s->buffer_ids));
        if (!s->buffer_ids)
            return -1;
    }

    for (int i = 0; i <s->nb_buffers; i++) {
        struct ngl_node *bnode = s->buffers[i];
        struct buffer *b = bnode->priv_data;
        ret = ngli_node_init(bnode);
        if (ret < 0)
            return ret;

        GLenum props[] = { GL_BUFFER_BINDING };
        GLsizei nb_props = 1;
        GLint params;
        GLsizei nb_params = 1;
        GLsizei nb_params_ret = 0;

        GLuint index = ngli_glGetProgramResourceIndex(gl,
                                                      shader->program_id,
                                                      GL_SHADER_STORAGE_BLOCK,
                                                      b->name);

        ngli_glGetProgramResourceiv(gl,
                                    shader->program_id,
                                    GL_SHADER_STORAGE_BLOCK,
                                    index,
                                    nb_props,
                                    props,
                                    nb_params,
                                    &nb_params_ret,
                                    &params);

        s->buffer_ids[i] = params;
    }

    if (s->nb_textures > glcontext->max_texture_image_units) {
        LOG(ERROR, "Attached textures count (%d) exceeds driver limit (%d)",
            s->nb_textures, glcontext->max_texture_image_units);
        return -1;
    }

    if (s->nb_textures) {
        s->textureshaderinfos = calloc(s->nb_textures, sizeof(*s->textureshaderinfos));
        if (!s->textureshaderinfos)
            return -1;
    }

    for (int i = 0; i < s->nb_textures; i++)  {
        char name[32];

        if (s->textures[i]) {
            ret = ngli_node_init(s->textures[i]);
            if (ret < 0)
                return ret;

            snprintf(name, sizeof(name), "img%d", i);
            s->textureshaderinfos[i].sampler_id = ngli_glGetUniformLocation(gl, shader->program_id, name);

            snprintf(name, sizeof(name), "img%d_dimensions", i);
            s->textureshaderinfos[i].dimensions_id = ngli_glGetUniformLocation(gl, shader->program_id, name);
        }
    }

    return 0;
}

static void compute_uninit(struct ngl_node *node)
{
    struct compute *s = node->priv_data;

    free(s->textureshaderinfos);
    free(s->uniform_ids);
}

static void compute_update(struct ngl_node *node, double t)
{
    struct compute *s = node->priv_data;

    for (int i = 0; i < s->nb_textures; i++) {
        if (s->textures[i]) {
            ngli_node_update(s->textures[i], t);
        }
    }

    for (int i = 0; i < s->nb_uniforms; i++) {
        ngli_node_update(s->uniforms[i], t);
    }

    ngli_node_update(s->shader, t);
}

static void compute_draw(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct compute *s = node->priv_data;
    const struct computeshader *shader = s->shader->priv_data;

    ngli_glUseProgram(gl, shader->program_id);

    update_uniforms(node);

    ngli_glDispatchCompute(gl, s->nb_group_x, s->nb_group_y, s->nb_group_z);

    ngli_glMemoryBarrier(gl, GL_ALL_BARRIER_BITS);
}

const struct node_class ngli_compute_class = {
    .id        = NGL_NODE_COMPUTE,
    .name      = "Compute",
    .init      = compute_init,
    .uninit    = compute_uninit,
    .update    = compute_update,
    .draw      = compute_draw,
    .priv_size = sizeof(struct compute),
    .params    = compute_params,
};
