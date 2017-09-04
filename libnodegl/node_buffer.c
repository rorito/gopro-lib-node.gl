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

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "nodegl.h"
#include "nodes.h"
#include "utils.h"

#define OFFSET(x) offsetof(struct buffer, x)
static const struct node_param buffer_params[] = {
    {"id",   PARAM_TYPE_STR,  OFFSET(name), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"n",    PARAM_TYPE_INT,  OFFSET(n), {.i64=1}, .flags=PARAM_FLAG_CONSTRUCTOR},
    {"data", PARAM_TYPE_DATA, OFFSET(data), {.p=NULL}},
    {NULL}
};

static int buffer_init(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct buffer *s = node->priv_data;

    switch (node->class->id) {
    case NGL_NODE_BUFFERFLOAT: s->data_comp = 1; break;
    case NGL_NODE_BUFFERVEC2:  s->data_comp = 2; break;
    case NGL_NODE_BUFFERVEC3:  s->data_comp = 3; break;
    case NGL_NODE_BUFFERVEC4:  s->data_comp = 4; break;
    default:
        ngli_assert(0);
    }

    s->data_stride = s->data_comp * sizeof(float);

    if (s->data) {
        if (s->data_size != s->n * s->data_stride)
            return -1;
    } else {
        s->data_size = s->n * s->data_stride;
        s->data = calloc(s->n, s->data_stride);
        if (!s->data)
            return -1;
    }

    ngli_glGenBuffers(gl, 1, &s->buffer_id);
    ngli_glBindBuffer(gl, GL_ARRAY_BUFFER, s->buffer_id);
    ngli_glBufferData(gl, GL_ARRAY_BUFFER, s->data_size, s->data, GL_DYNAMIC_COPY);
    ngli_glBindBuffer(gl, GL_ARRAY_BUFFER, 0);

    return 0;
}

static void buffer_uninit(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct buffer *s = node->priv_data;

    ngli_glDeleteBuffers(gl, 1, &s->buffer_id);

    free(s->data);
}

const struct node_class ngli_bufferfloat_class = {
    .id        = NGL_NODE_BUFFERFLOAT,
    .name      = "BufferFloat",
    .init      = buffer_init,
    .uninit    = buffer_uninit,
    .priv_size = sizeof(struct buffer),
    .params    = buffer_params,
};

const struct node_class ngli_buffervec2_class = {
    .id        = NGL_NODE_BUFFERVEC2,
    .name      = "BufferVec2",
    .init      = buffer_init,
    .uninit    = buffer_uninit,
    .priv_size = sizeof(struct buffer),
    .params    = buffer_params,
};

const struct node_class ngli_buffervec3_class = {
    .id        = NGL_NODE_BUFFERVEC3,
    .name      = "BufferVec3",
    .init      = buffer_init,
    .uninit    = buffer_uninit,
    .priv_size = sizeof(struct buffer),
    .params    = buffer_params,
};

const struct node_class ngli_buffervec4_class = {
    .id        = NGL_NODE_BUFFERVEC4,
    .name      = "BufferVec4",
    .init      = buffer_init,
    .uninit    = buffer_uninit,
    .priv_size = sizeof(struct buffer),
    .params    = buffer_params,
};
