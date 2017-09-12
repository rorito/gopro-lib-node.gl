/*
 * Copyright 2016 GoPro Inc.
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
#include "math_utils.h"

#define OFFSET(x) offsetof(struct rotate, x)
static const struct node_param rotate_params[] = {
    {"child", PARAM_TYPE_NODE, OFFSET(child), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"angle", PARAM_TYPE_DBL,  OFFSET(angle)},
    {"axis",  PARAM_TYPE_VEC3, OFFSET(axis), {.vec={0.0, 0.0, 1.0}}},
    {"anchor", PARAM_TYPE_VEC3, OFFSET(anchor), {.vec={0.0, 0.0, 0.0}}},
    {"anim",   PARAM_TYPE_NODE, OFFSET(anim), .node_types=(const int[]){NGL_NODE_ANIMATIONSCALAR, -1}},
    {NULL}
};

static const float get_angle(struct rotate *s, double t)
{
    if (!s->anim)
        return s->angle;
    struct ngl_node *anim_node = s->anim;
    struct animation *anim = anim_node->priv_data;
    ngli_node_update(anim_node, t);
    return anim->values[0];
}

static void rotate_update(struct ngl_node *node, double t)
{
    struct rotate *s = node->priv_data;
    struct ngl_node *child = s->child;
    NGLI_ALIGNED_MAT(trans);
    const float x = get_angle(s, t) * 2.0f * M_PI / 360.0f;
    static const float zero_anchor[3] = { 0.0, 0.0, 0.0 };
    int translate = memcmp(s->anchor, zero_anchor, sizeof(s->anchor));

    if (translate) {
        const NGLI_ALIGNED_MAT(transm) = {
            1.0f,   0.0f,   0.0f,   0.0f,
            0.0f,   1.0f,   0.0f,   0.0f,
            0.0f,   0.0f,   1.0f,   0.0f,
            s->anchor[0], s->anchor[1], s->anchor[2], 1.0f,
        };
        ngli_mat4_mul(trans, node->modelview_matrix, transm);
    } else {
        memcpy(trans, node->modelview_matrix, sizeof(node->modelview_matrix));
    }

    if (s->axis[0] == 1) {
        const NGLI_ALIGNED_MAT(rotm) = {
            1.0f,    0.0f,    0.0f, 0.0f,
            0.0f,  cos(x),  sin(x), 0.0f,
            0.0f, -sin(x),  cos(x), 0.0f,
            0.0f,    0.0f,    0.0f, 1.0f,
        };
        ngli_mat4_mul(child->modelview_matrix, trans, rotm);
    } else if (s->axis[1] == 1) {
        const NGLI_ALIGNED_MAT(rotm) = {
            cos(x), 0.0f, -sin(x), 0.0f,
              0.0f, 1.0f,    0.0f, 0.0f,
            sin(x), 0.0f,  cos(x), 0.0f,
              0.0f, 0.0f,    0.0f, 1.0f,
        };
        ngli_mat4_mul(child->modelview_matrix, trans, rotm);
    } else if (s->axis[2] == 1) {
        const NGLI_ALIGNED_MAT(rotm) = {
            cos(x),  sin(x), 0.0f, 0.0f,
           -sin(x),  cos(x), 0.0f, 0.0f,
              0.0f,    0.0f, 1.0f, 0.0f,
              0.0f,    0.0f, 0.0f, 1.0f,
        };
        ngli_mat4_mul(child->modelview_matrix, trans, rotm);
    }

    if (translate) {
        const NGLI_ALIGNED_MAT(transm) = {
            1.0f,   0.0f,   0.0f,   0.0f,
            0.0f,   1.0f,   0.0f,   0.0f,
            0.0f,   0.0f,   1.0f,   0.0f,
            -s->anchor[0], -s->anchor[1], -s->anchor[2], 1.0f,
        };
        ngli_mat4_mul(child->modelview_matrix, child->modelview_matrix, transm);
    }

    memcpy(child->projection_matrix, node->projection_matrix, sizeof(node->projection_matrix));
    ngli_node_update(child, t);
}

static void rotate_draw(struct ngl_node *node)
{
    struct rotate *s = node->priv_data;
    ngli_node_draw(s->child);
}

const struct node_class ngli_rotate_class = {
    .id        = NGL_NODE_ROTATE,
    .name      = "Rotate",
    .update    = rotate_update,
    .draw      = rotate_draw,
    .priv_size = sizeof(struct rotate),
    .params    = rotate_params,
};
