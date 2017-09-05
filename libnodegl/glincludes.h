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

#ifndef GLINCLUDES_H
#define GLINCLUDES_H

#if __APPLE__
# include <TargetConditionals.h>
# if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#  include <OpenGLES/ES2/gl.h>
#  include <OpenGLES/ES2/glext.h>
#  define GL_MAJOR_VERSION  0x821B
#  define GL_MINOR_VERSION  0x821C
#  define GL_NUM_EXTENSIONS 0x821D
#  define GL_RED            GL_LUMINANCE
#  define GL_R32F           0x822E
#  define GL_READ_ONLY      0x88B8
#  define GL_WRITE_ONLY     0x88B9
#  define GL_READ_WRITE     0x88BA
#  define GL_COMPUTE_SHADER                     0x91B9
#  define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#  define GL_MAX_COMPUTE_WORK_GROUP_COUNT       0x91BE
#  define GL_MAX_COMPUTE_WORK_GROUP_SIZE        0x91BF
#  define GL_COMPUTE_WORK_GROUP_SIZE            0x8267
#  define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT    0x00000020
# elif TARGET_OS_MAC
#  include <OpenGL/gl3.h>
#  include <OpenGL/glext.h>
# endif
#endif

#if __ANDROID__
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
# define GL_BGRA           GL_BGRA_EXT
# define GL_MAJOR_VERSION  0x821B
# define GL_MINOR_VERSION  0x821C
# define GL_NUM_EXTENSIONS 0x821D
# define GL_RED            GL_LUMINANCE
# define GL_R32F           0x822E
# define GL_READ_ONLY      0x88B8
# define GL_WRITE_ONLY     0x88B9
# define GL_READ_WRITE     0x88BA
# define GL_COMPUTE_SHADER                     0x91B9
# define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
# define GL_MAX_COMPUTE_WORK_GROUP_COUNT       0x91BE
# define GL_MAX_COMPUTE_WORK_GROUP_SIZE        0x91BF
# define GL_COMPUTE_WORK_GROUP_SIZE            0x8267
# define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT    0x00000020
#endif

#if __linux__ && !__ANDROID__
# define GL_GLEXT_PROTOTYPES
# include <GL/glcorearb.h>
# include <GL/glext.h>
#endif

#if _WIN32
# include <GL/gl.h>
# include <GL/glcorearb.h>
# include <GL/glext.h>
#endif

#endif /* GLINCLUDES_H */
