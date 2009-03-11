/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_x11gl_c.h,v 1.8 2004/01/04 16:49:27 slouken Exp $";
#endif

#ifdef SDL_VIDEO_OPENGL_ES
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <dlfcn.h>
#if defined(__OpenBSD__) && !defined(__ELF__)
#define dlsym(x,y) dlsym(x, "_" y)
#endif
#endif
#include "../SDL_sysvideo.h"

struct SDL_PrivateGLESData {
    int egl_active; /* to stop switching drivers while we have a valid context */
    XVisualInfo* egl_visualinfo;

#ifdef SDL_VIDEO_OPENGL_ES
    EGLDisplay egl_display;
    EGLContext egl_context;	/* Current GLES context */
    EGLSurface egl_surface;
    EGLConfig  egl_config;

    EGLDisplay (*eglGetDisplay) (NativeDisplayType display);
    EGLBoolean (*eglInitialize) (EGLDisplay dpy, EGLint *major, EGLint *minor);
    EGLBoolean (*eglTerminate) (EGLDisplay dpy);

    void * (*eglGetProcAddress)(const GLubyte *procName);

    EGLBoolean (*eglChooseConfig) (EGLDisplay dpy,
                                   const EGLint *attrib_list,
                                   EGLConfig *configs,
                                   EGLint config_size,
                                   EGLint *num_config);

    EGLContext (*eglCreateContext) (EGLDisplay dpy,
                                    EGLConfig config,
                                    EGLContext share_list,
                                    const EGLint *attrib_list);

    EGLBoolean (*eglDestroyContext) (EGLDisplay dpy, EGLContext ctx);

    EGLSurface (*eglCreateWindowSurface) (EGLDisplay dpy,
                                          EGLConfig config,
                                          NativeWindowType window,
                                          const EGLint *attrib_list);
    EGLBoolean (*eglDestroySurface) (EGLDisplay dpy, EGLSurface surface);

    EGLBoolean (*eglMakeCurrent) (EGLDisplay dpy, EGLSurface draw,
                                  EGLSurface read, EGLContext ctx);

    EGLBoolean (*eglSwapBuffers) (EGLDisplay dpy, EGLSurface draw);

    const char *(*eglQueryString) (EGLDisplay dpy, EGLint name);

    EGLBoolean (*eglGetConfigAttrib) (EGLDisplay dpy, EGLConfig config,
                                      EGLint attribute, EGLint *value);

#endif /* SDL_VIDEO_OPENGL_ES */
};

/* OpenGL functions */
extern int X11_GLES_CreateWindow(_THIS, int w, int h);
extern int X11_GLES_CreateContext(_THIS);
extern void X11_GLES_Shutdown(_THIS);
#ifdef SDL_VIDEO_OPENGL_ES
extern XVisualInfo *X11_GLES_GetVisual(_THIS);
extern int X11_GLES_MakeCurrent(_THIS);
extern int X11_GLES_GetAttribute(_THIS, SDL_GLattr attrib, int* value);
extern void X11_GLES_SwapBuffers(_THIS);
extern int X11_GLES_LoadLibrary(_THIS, const char* path);
extern void *X11_GLES_GetProcAddress(_THIS, const char* proc);
#else
/* Dummy EGL types */
typedef int EGLConfig;
typedef int EGLint;
#endif
extern void X11_GLES_UnloadLibrary(_THIS);

