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

#include <stdarg.h>
#include <stdio.h>

#include "log.h"

struct log_ctx {
    void *user_arg;
    ngl_log_callback_type callback;
    int min_level;
};

static void default_callback(void *arg, int level, const char *module,
                             const char *filename, int ln, const char *fn,
                             const char *fmt, va_list vl)
{
    char logline[512];
    static const char * const log_strs[] = {
        [NGL_LOG_DEBUG]   = "DEBUG",
        [NGL_LOG_VERBOSE] = "VERBOSE",
        [NGL_LOG_INFO]    = "INFO",
        [NGL_LOG_WARNING] = "WARNING",
        [NGL_LOG_ERROR]   = "ERROR",
    };
    vsnprintf(logline, sizeof(logline), fmt, vl);
    printf("[%s] %s @ %s:%d %s: %s\n", log_strs[level],
           module, filename, ln, fn, logline);
}

struct log_ctx *ngli_log_create(void)
{
    struct log_ctx *log = calloc(1, sizeof(*log));
    if (!log)
        return NULL;
    log->callback  = default_callback;
    log->min_level = NGL_LOG_INFO;
    return log;
}

void ngli_log_set_callback(struct log_ctx *log, void *arg,
                           ngl_log_callback_type callback)
{
    log->user_arg = arg;
    log->callback = callback;
}

void ngli_log_set_min_level(struct log_ctx *log, int level)
{
    log->min_level = level;
}

void ngli_log(struct log_ctx *log, int log_level,
              const char *module, const char *filename,
              int ln, const char *fn, const char *fmt, ...)
{
    va_list arg_list;

    if (log_level < log->min_level)
        return;

    va_start(arg_list, fmt);
    log->callback(log->user_arg, log_level, module,
                  filename, ln, fn, fmt, arg_list);
    va_end(arg_list);
}
