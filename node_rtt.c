/*
 * Copyright 2016 Clément Bœsch <cboesch@gopro.com>
 * Copyright 2016 Matthieu Bouron <mbouron@gopro.com>
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

#include "log.h"
#include "nodegl.h"
#include "nodes.h"
#include "utils.h"

#define OFFSET(x) offsetof(struct rtt, x)
static const struct node_param rtt_params[] = {
    {"child",   PARAM_TYPE_NODE, OFFSET(child),   .flags=PARAM_FLAG_CONSTRUCTOR},
    {"color_texture", PARAM_TYPE_NODE, OFFSET(color_texture), .flags=PARAM_FLAG_CONSTRUCTOR,
                      .node_types=(const int[]){NGL_NODE_TEXTURE, -1}},
    {"depth_texture", PARAM_TYPE_NODE, OFFSET(depth_texture),
                      .node_types=(const int[]){NGL_NODE_TEXTURE, -1}},
    {NULL}
};

static int rtt_init(struct ngl_node *node)
{
    struct rtt *s = node->priv_data;
    struct texture *t = s->color_texture->priv_data;
    struct texture *d = NULL;

    ngli_node_init(s->color_texture);
    s->width = t->width;
    s->height = t->height;

    if (s->depth_texture) {
        d = s->depth_texture->priv_data;
        ngli_node_init(s->depth_texture);
        ngli_assert(s->width == d->width && s->height == d->height);
    }

    GLuint framebuffer_id = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&framebuffer_id);

    glGenFramebuffers(1, &s->framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, s->framebuffer_id);

    LOG(VERBOSE, "init rtt with texture %d", t->id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->id, 0);

    if (d) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d->id, 0);
    } else {
        glGenRenderbuffers(1, &s->renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, s->renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, s->width, s->height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s->renderbuffer_id);
    }

    ngli_assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    ngli_node_init(s->child);

    return 0;
}

static void rtt_update(struct ngl_node *node, double t)
{
    struct rtt *s = node->priv_data;
    ngli_node_update(s->child, t);
    ngli_node_update(s->color_texture, t);
}

static void rtt_draw(struct ngl_node *node)
{
    GLint viewport[4];
    struct rtt *s = node->priv_data;

    GLuint framebuffer_id = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, s->framebuffer_id);

    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, s->width, s->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ngli_node_draw(s->child);

    ngli_assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    struct texture *texture = s->color_texture->priv_data;
    switch(texture->min_filter) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
        glBindTexture(GL_TEXTURE_2D, texture->id);
        glGenerateMipmap(GL_TEXTURE_2D);
        break;
    }
}

static void rtt_uninit(struct ngl_node *node)
{
    struct rtt *s = node->priv_data;

    glBindFramebuffer(GL_FRAMEBUFFER, s->framebuffer_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, 0);

    glDeleteRenderbuffers(1, &s->renderbuffer_id);
    glDeleteFramebuffers(1, &s->framebuffer_id);
}

const struct node_class ngli_rtt_class = {
    .id        = NGL_NODE_RTT,
    .name      = "RTT",
    .init      = rtt_init,
    .update    = rtt_update,
    .draw      = rtt_draw,
    .uninit    = rtt_uninit,
    .priv_size = sizeof(struct rtt),
    .params    = rtt_params,
};