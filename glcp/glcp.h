/*
 * glcp
 * version 0.0.0
 * supported OpenGL version 0.0
 *
 * Copyright (C) 2013-2026 Shun Moriya
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * generate from glcp.rb at 2026-05-14 15:34:28
 */

#if !defined(___GL_CORE_PROFILE_H___)
#define ___GL_CORE_PROFILE_H___
#include <windows.h>
#include <gl/gl.h>
#if defined(GL_VERSION_1_1) && !defined(GL_VERSION_1_0)
#define GL_VERSION_1_0
#endif
#if defined(__glext_h_)
#error glext.h included before glcp.h
#endif
#if defined(__wglext_h_)
#error wglext.h included before glcp.h
#endif
#if defined(__glxext_h_)
#error glxext.h included before glcp.h
#endif
/* <-- glcorearb.h */

Download "glcorearb.h" from https://www.opengl.org/registry/
/* --> glcorearb.h */
#if defined(__cplusplus)
extern "C" {
#endif
extern void glcpInitialize();
extern void glcpFinalize();
#if defined(__cplusplus)
}
#endif
#endif /*___GL_CORE_PROFILE_H___*/
