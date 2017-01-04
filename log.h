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

#ifndef LOG_H
#define LOG_H

#include "nodegl.h"
#include "utils.h"

#define NGLI_LOG(logger, module, log_level, ...) ngli_log(logger, NGL_LOG_##log_level, \
                                                          module, __FILE__, __LINE__,  \
                                                          __FUNCTION__, __VA_ARGS__)

struct log_ctx *ngli_log_create(void);

void ngli_log_set_callback(struct log_ctx *log, void *arg,
                           ngl_log_callback_type callback);

void ngli_log_set_min_level(struct log_ctx *log, int level);

void ngli_log(struct log_ctx *log, int log_level,
              const char *module, const char *filename,
              int ln, const char *fn, const char *fmt, ...) ngli_printf_format(7, 8);

#endif /* LOG_H */
