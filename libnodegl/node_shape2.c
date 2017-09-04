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

#include <stdlib.h>
#include "nodegl.h"
#include "nodes.h"

#define SET_INDICES(type) do {                                                 \
    type *indices = (type *)s->indices;                                        \
    for (int i = 0; i < s->nb_indices; i++) {                                  \
        indices[i] = i;                                                        \
    }                                                                          \
} while (0)

#define OFFSET(x) offsetof(struct shape, x)
static const struct node_param shape2_params[] = {
    {"vertices",  PARAM_TYPE_NODE, OFFSET(vertices_buffer),  .node_types=(const int[]){NGL_NODE_BUFFERVEC3, -1}, .flags=PARAM_FLAG_CONSTRUCTOR},
    {"texcoords", PARAM_TYPE_NODE, OFFSET(texcoords_buffer), .node_types=(const int[]){NGL_NODE_BUFFERVEC2, -1}},
    {"normals",   PARAM_TYPE_NODE, OFFSET(normals_buffer),   .node_types=(const int[]){NGL_NODE_BUFFERVEC3, -1}},
    {"draw_mode", PARAM_TYPE_INT, OFFSET(draw_mode), {.i64=GL_TRIANGLES}},
    {"draw_type", PARAM_TYPE_INT, OFFSET(draw_type), {.i64=GL_UNSIGNED_SHORT}},
    {NULL}
};

static int shape2_init(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct shape *s = node->priv_data;
    struct buffer *vertices = s->vertices_buffer->priv_data;

    s->positions_stride = sizeof(float[3]);
    s->texcoords_stride = sizeof(float[2]);
    s->normals_stride = sizeof(float[3]);

    s->vertices_buffer_id = vertices->buffer_id;

    if (s->texcoords_buffer) {
        struct buffer *b = s->texcoords_buffer->priv_data;
        if (b->n != s->nb_vertices)
            return -1;
        s->texcoords_buffer_id = b->buffer_id;
    }

    if (s->normals_buffer) {
        struct buffer *b = s->normals_buffer->priv_data;
        if (b->n != s->nb_vertices)
            return -1;
        s->normals_buffer_id = b->buffer_id;
    }

    switch(s->draw_type) {
    case GL_UNSIGNED_BYTE:  s->indice_size = sizeof(GLubyte);  break;
    case GL_UNSIGNED_SHORT: s->indice_size = sizeof(GLushort); break;
    case GL_UNSIGNED_INT:   s->indice_size = sizeof(GLuint);   break;
    default:
        ngli_assert(0);
    }

    s->nb_indices = vertices->n;
    s->indices = calloc(s->nb_indices, s->indice_size);
    if (!s->indices)
        return -1;

    switch(s->draw_type) {
    case GL_UNSIGNED_BYTE:  SET_INDICES(GLubyte);  break;
    case GL_UNSIGNED_SHORT: SET_INDICES(GLushort); break;
    case GL_UNSIGNED_INT:   SET_INDICES(GLuint);   break;
    default:
        ngli_assert(0);
    }

    ngli_glGenBuffers(gl, 1, &s->indices_buffer_id);
    ngli_glBindBuffer(gl, GL_ELEMENT_ARRAY_BUFFER, s->indices_buffer_id);
    ngli_glBufferData(gl, GL_ELEMENT_ARRAY_BUFFER,
                      s->nb_indices * s->indice_size, s->indices, GL_STATIC_DRAW);
    ngli_glBindBuffer(gl, GL_ELEMENT_ARRAY_BUFFER, 0);

    return 0;
}

static void shape2_uninit(struct ngl_node *node)
{
    struct shape *s = node->priv_data;

    free(s->indices);
}

const struct node_class ngli_shape2_class = {
    .id        = NGL_NODE_SHAPE2,
    .name      = "Shape2",
    .init      = shape2_init,
    .uninit    = shape2_uninit,
    .priv_size = sizeof(struct shape),
    .params    = shape2_params,
};
