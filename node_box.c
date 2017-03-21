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
#include "gl_utils.h"
#include "nodegl.h"
#include "nodes.h"
#include "utils.h"

#define OFFSET(x) offsetof(struct shape, x)
static const struct node_param box_params[] = {
    {"corner", PARAM_TYPE_VEC3, OFFSET(box_corner), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"width",  PARAM_TYPE_VEC3, OFFSET(box_width),  .flags=PARAM_FLAG_CONSTRUCTOR},
    {"height", PARAM_TYPE_VEC3, OFFSET(box_height), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"depth",  PARAM_TYPE_VEC3, OFFSET(box_depth),  .flags=PARAM_FLAG_CONSTRUCTOR},
    {"uv_corner", PARAM_TYPE_VEC2, OFFSET(box_uv_corner), {.vec={0.0f, 0.0f}}},
    {"uv_width",  PARAM_TYPE_VEC2, OFFSET(box_uv_width),  {.vec={1.0f, 0.0f}}},
    {"uv_height", PARAM_TYPE_VEC2, OFFSET(box_uv_height), {.vec={0.0f, 1.0f}}},
    {NULL}
};

#define C(index) s->box_corner[(index)]
#define W(index) s->box_width[(index)]
#define H(index) s->box_height[(index)]
#define D(index) s->box_depth[(index)]

#define UV_C(index) s->box_uv_corner[(index)]
#define UV_W(index) s->box_uv_width[(index)]
#define UV_H(index) s->box_uv_height[(index)]

static int box_init(struct ngl_node *node)
{
    struct shape *s = node->priv_data;

    const GLfloat vertices[] = {
        C(0) + W(0) + D(0),        C(1) + W(1) + D(1),        C(2) + W(2) - D(2),        UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0) + D(0),               C(1) + D(1),               C(2) - D(2),               UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + H(0) + D(0),        C(1) + H(1) + D(1),        C(2) + H(2) - D(2),        UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0) + D(0), C(1) + H(1) + W(1) + D(1), C(2) + H(2) + W(2) - D(2), UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,

        C(0) + D(0),        C(1) + D(1),        C(2) - D(2),                             UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0),               C(1),               C(2),                                    UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + H(0),        C(1) + H(1),        C(2) + H(2),                             UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0) + H(0) + D(0), C(1) + H(1) + D(1), C(2) + H(2) - D(2),                      UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,

        C(0) + W(0),               C(1) + W(1),               C(2) + W(2),               UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0) + W(0) + D(0),        C(1) + W(1) + D(1),        C(2) + W(2) - D(2),        UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0) + D(0), C(1) + H(1) + W(1) + D(1), C(2) + H(2) + W(2) - D(2), UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0),        C(1) + H(1) + W(1),        C(2) + H(2) + W(2),        UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,


        C(0) + D(0),        C(1) + D(1),        C(2) - D(2),                             UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0) + W(0) + D(0), C(1) + W(1) + D(1), C(2) + W(2) - D(2),                      UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + W(0),        C(1) + W(1),        C(2) + W(2),                             UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0),               C(1),               C(2),                                    UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,


        C(0) + H(0),               C(1) + H(1),               C(2) + H(2),               UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0),        C(1) + H(1) + W(1),        C(2) + H(2) + W(2),        UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0) + D(0), C(1) + H(1) + W(1) + D(1), C(2) + H(2) + W(2) - D(2), UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0) + H(0) + D(0),        C(1) + H(1) + D(1),        C(2) + H(2) - D(2),        UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,



        C(0),               C(1),               C(2),                                    UV_C(0),                     1.0f - UV_C(1),                     0.0f, 0.0f, 0.0f,
        C(0) + W(0),        C(1) + W(1),        C(2) + W(2),                             UV_C(0) + UV_W(0),           1.0f - UV_C(1) - UV_W(1),           0.0f, 0.0f, 0.0f,
        C(0) + H(0) + W(0), C(1) + H(1) + W(1), C(2) + H(2) + W(2),                      UV_C(0) + UV_H(0) + UV_W(0), 1.0f - UV_C(1) - UV_H(1) - UV_W(1), 0.0f, 0.0f, 0.0f,
        C(0) + H(0),        C(1) + H(1),        C(2) + H(2),                             UV_C(0) + UV_H(0),           1.0f - UV_C(1) - UV_H(1),           0.0f, 0.0f, 0.0f,
    };
    s->nb_vertices = sizeof(vertices) / NGLI_SHAPE_VERTICES_STRIDE(s);
    s->vertices = calloc(1, sizeof(vertices));
    if (!s->vertices)
        return -1;

    memcpy(s->vertices, vertices, sizeof(vertices));
    NGLI_SHAPE_GENERATE_BUFFERS(s);

    static const GLushort indices[] = {
        0,  1,  2,  0,  2,  3,
        4,  5,  6,  4,  6,  7,
        8,  9,  10, 8,  10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    s->nb_indices = NGLI_ARRAY_NB(indices);
    s->indices = calloc(1, sizeof(indices));
    if (!s->indices)
        return -1;

    memcpy(s->indices, indices, sizeof(indices));
    NGLI_SHAPE_GENERATE_ELEMENT_BUFFERS(s);

    s->draw_mode = GL_TRIANGLES;
    s->draw_type = GL_UNSIGNED_SHORT;

    return 0;
}

static void box_uninit(struct ngl_node *node)
{
    struct shape *s = node->priv_data;

    free(s->vertices);
    free(s->indices);
}

const struct node_class ngli_box_class = {
    .id        = NGL_NODE_BOX,
    .name      = "Box",
    .init      = box_init,
    .uninit    = box_uninit,
    .priv_size = sizeof(struct shape),
    .params    = box_params,
};
