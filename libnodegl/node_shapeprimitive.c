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
#include "utils.h"

#define OFFSET(x) offsetof(struct shapeprimitive, x)
static const struct node_param shapeprimitive_params[] = {
    {"coordinates",         PARAM_TYPE_VEC3, OFFSET(coordinates), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"texture_coordinates", PARAM_TYPE_VEC2, OFFSET(texture_coordinates), .flags=PARAM_FLAG_CONSTRUCTOR},
    {"normals",             PARAM_TYPE_VEC3, OFFSET(normals)},
    {"animkf_x",            PARAM_TYPE_NODELIST, OFFSET(coords[0].animkf), .flags=PARAM_FLAG_DOT_DISPLAY_PACKED,
                            .node_types=(const int[]){NGL_NODE_ANIMKEYFRAMESCALAR, -1}},
    {"animkf_y",            PARAM_TYPE_NODELIST, OFFSET(coords[1].animkf), .flags=PARAM_FLAG_DOT_DISPLAY_PACKED,
                            .node_types=(const int[]){NGL_NODE_ANIMKEYFRAMESCALAR, -1}},
    {"animkf_z",            PARAM_TYPE_NODELIST, OFFSET(coords[2].animkf), .flags=PARAM_FLAG_DOT_DISPLAY_PACKED,
                            .node_types=(const int[]){NGL_NODE_ANIMKEYFRAMESCALAR, -1}},
    {NULL}
};

static int shapeprimitive_init(struct ngl_node *node)
{
    struct shapeprimitive *s = node->priv_data;
    for (int i = 0; i < 3; i++)
        s->dynamic_coords |= !!s->coords[i].nb_animkf;
    return 0;
}

static void shapeprimitive_update(struct ngl_node *node, double t)
{
    struct shapeprimitive *s = node->priv_data;
    for (int i = 0; i < 3; i++)
        if (s->coords[i].nb_animkf)
            ngli_animkf_interpolate(&s->coords[i].offset,
                                    s->coords[i].animkf,
                                    s->coords[i].nb_animkf,
                                    &s->coords[i].current_kf, t);
}

const struct node_class ngli_shapeprimitive_class = {
    .id        = NGL_NODE_SHAPEPRIMITIVE,
    .name      = "ShapePrimitive",
    .init      = shapeprimitive_init,
    .update    = shapeprimitive_update,
    .priv_size = sizeof(struct shapeprimitive),
    .params    = shapeprimitive_params,
};
