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

#ifndef PARAMS_H
#define PARAMS_H

#include <stdarg.h>

#include "bstr.h"

enum {
    PARAM_TYPE_INT,
    PARAM_TYPE_I64,
    PARAM_TYPE_DBL,
    PARAM_TYPE_STR,
    PARAM_TYPE_VEC2,
    PARAM_TYPE_VEC3,
    PARAM_TYPE_VEC4,
    PARAM_TYPE_NODE,
    PARAM_TYPE_NODELIST,
    PARAM_TYPE_DBLLIST,
    PARAM_TYPE_DATA,
};

#define PARAM_FLAG_CONSTRUCTOR (1<<0)
#define PARAM_FLAG_DOT_DISPLAY_PACKED (1<<1)
#define PARAM_FLAG_DOT_DISPLAY_FIELDNAME (1<<2)
struct node_param {
    const char *key;
    int type;
    int offset;
    union {
        int64_t i64;
        double dbl;
        const char *str;
        void *p;
        float vec[4];
    } def_value;
    int flags;
    const int *node_types;
};

const struct node_param *ngli_params_find(const struct node_param *params, const char *key);
void ngli_params_bstr_print_val(struct bstr *b, uint8_t *base_ptr, const struct node_param *par);
int ngli_params_set(uint8_t *base_ptr, const struct node_param *par, va_list *ap);
int ngli_params_vset(uint8_t *base_ptr, const struct node_param *par, ...);
int ngli_params_set_constructors(uint8_t *base_ptr, const struct node_param *params, va_list *ap);
int ngli_params_set_defaults(uint8_t *base_ptr, const struct node_param *params);
int ngli_params_add(uint8_t *base_ptr, const struct node_param *par, int nb_elems, void *elems);
void ngli_params_free(uint8_t *base_ptr, const struct node_param *params);

#endif
