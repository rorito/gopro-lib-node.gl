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
#include "bstr.h"
#include "glincludes.h"
#include "log.h"
#include "nodegl.h"
#include "nodes.h"

#define OFFSET(x) offsetof(struct computeshader, x)
static const struct node_param computeshader_params[] = {
    {"compute_data", PARAM_TYPE_STR, OFFSET(compute_data), .flags=PARAM_FLAG_CONSTRUCTOR},
    {NULL}
};

#define DEFINE_GET_INFO_LOG_FUNCTION(func, name)                                      \
static void get_##func##_info_log(const struct glfunctions *gl, GLuint id,            \
                                  char **info_logp, int *info_log_lengthp)            \
{                                                                                     \
    ngli_glGet##name##iv(gl, id, GL_INFO_LOG_LENGTH, info_log_lengthp);               \
    if (!*info_log_lengthp) {                                                         \
        *info_logp = NULL;                                                            \
        return;                                                                       \
    }                                                                                 \
                                                                                      \
    *info_logp = malloc(*info_log_lengthp);                                           \
    if (!*info_logp) {                                                                \
        *info_log_lengthp = 0;                                                        \
        return;                                                                       \
    }                                                                                 \
                                                                                      \
    ngli_glGet##name##InfoLog(gl, id, *info_log_lengthp, NULL, *info_logp);           \
    while (*info_log_lengthp && strchr(" \r\n", (*info_logp)[*info_log_lengthp - 1])) \
        (*info_logp)[--*info_log_lengthp] = 0;                                        \
}                                                                                     \

DEFINE_GET_INFO_LOG_FUNCTION(shader, Shader)
DEFINE_GET_INFO_LOG_FUNCTION(program, Program)

static GLuint load_shader(struct ngl_node *node, const char *compute_shader_data)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    char *info_log = NULL;
    int info_log_length = 0;

    GLint result = GL_FALSE;

    GLuint program = ngli_glCreateProgram(gl);
    GLuint compute_shader = ngli_glCreateShader(gl, GL_COMPUTE_SHADER);

    ngli_glShaderSource(gl, compute_shader, 1, &compute_shader_data, NULL);
    ngli_glCompileShader(gl, compute_shader);

    ngli_glGetShaderiv(gl, compute_shader, GL_COMPILE_STATUS, &result);
    if (!result) {
        get_shader_info_log(gl, compute_shader, &info_log, &info_log_length);
        goto fail;
    }

    ngli_glAttachShader(gl, program, compute_shader);
    ngli_glLinkProgram(gl, program);

    ngli_glGetProgramiv(gl, program, GL_LINK_STATUS, &result);
    if (!result) {
        get_program_info_log(gl, program, &info_log, &info_log_length);
        goto fail;
    }

    ngli_glDeleteShader(gl, compute_shader);

    return program;

fail:
    if (info_log) {
        LOG(ERROR, "could not compile or link shader: %s", info_log);
        free(info_log);
    }

    if (compute_shader) {
        ngli_glDeleteShader(gl, compute_shader);
    }

    if (program) {
        ngli_glDeleteProgram(gl, program);
    }

    return 0;
}

static int computeshader_init(struct ngl_node *node)
{
    struct computeshader *s = node->priv_data;

    s->program_id = load_shader(node, s->compute_data);
    if (!s->program_id)
        return -1;

    return 0;
}

static void computeshader_uninit(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    struct glcontext *glcontext = ctx->glcontext;
    const struct glfunctions *gl = &glcontext->funcs;

    struct computeshader *s = node->priv_data;

    ngli_glDeleteProgram(gl, s->program_id);
}

const struct node_class ngli_computeshader_class = {
    .id        = NGL_NODE_COMPUTESHADER,
    .name      = "ComputeShader",
    .init      = computeshader_init,
    .uninit    = computeshader_uninit,
    .priv_size = sizeof(struct computeshader),
    .params    = computeshader_params,
};
