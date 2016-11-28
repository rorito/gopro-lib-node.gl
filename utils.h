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

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUC__
# define ngli_printf_format(fmtpos, attrpos) __attribute__((__format__(__printf__, fmtpos, attrpos)))
#else
# define ngli_printf_format(fmtpos, attrpos)
#endif

#define ngli_assert(cond) do {                          \
    if (!(cond)) {                                      \
        fprintf(stderr, "Assert " #cond " @ %s:%d\n",   \
                __FILE__, __LINE__);                    \
        abort();                                        \
    }                                                   \
} while (0)

#define NGLI_ARRAY_NB(x) ((int)(sizeof(x)/sizeof(*(x))))
#define NGLI_SWAP(type, a, b) do { type tmp_swap = b; b = a; a = tmp_swap; } while (0)

char *ngli_strdup(const char *s);
int64_t ngli_gettime(void);
char *ngli_asprintf(const char *fmt, ...) ngli_printf_format(1, 2);

#endif /* UTILS_H */