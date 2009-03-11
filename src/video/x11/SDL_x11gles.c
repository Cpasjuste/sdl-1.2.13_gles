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
 "@(#) $Id: SDL_x11gl.c,v 1.19 2004/02/26 20:49:45 slouken Exp $";
#endif

#include <stdlib.h>	/* For getenv() prototype */
#include <string.h>

#include "../../events/SDL_events_c.h"
#include "SDL_error.h"
#include "SDL_x11video.h"
#include "SDL_x11dga_c.h"
#include "SDL_x11gles_c.h"

#define DEFAULT_OPENGL	"libGLES_CM.so"

/* return the preferred visual to use for openGL graphics */
XVisualInfo *X11_GLES_GetVisual(_THIS)
{
#ifdef SDL_VIDEO_OPENGL_ES
	/* 64 seems nice. */
	EGLint attribs[64];
	EGLint found_configs = 0;
	VisualID visual_id;
	int i;

	/* load the gl driver from a default path */
	if ( ! this->gl_config.driver_loaded ) {
	        /* no driver has been loaded, use default (ourselves) */
	        if ( X11_GLES_LoadLibrary(this, NULL) < 0 ) {
		        return NULL;
		}
	}

	/* See if we already have a window which we must use */
	if ( SDL_windowid ) {
		XWindowAttributes a;
		XVisualInfo vi_in;
		int out_count;

		XGetWindowAttributes(SDL_Display, SDL_Window, &a);
		vi_in.screen = SDL_Screen;
		vi_in.visualid = XVisualIDFromVisual(a.visual);
		this->gles_data->egl_visualinfo = XGetVisualInfo(SDL_Display,
			VisualScreenMask|VisualIDMask, &vi_in, &out_count);
		return this->gles_data->egl_visualinfo;
	}

        /* Setup our GLX attributes according to the gl_config. */
	i = 0;
	attribs[i++] = EGL_RED_SIZE;
	attribs[i++] = this->gl_config.red_size;
	attribs[i++] = EGL_GREEN_SIZE;
	attribs[i++] = this->gl_config.green_size;
	attribs[i++] = EGL_BLUE_SIZE;
	attribs[i++] = this->gl_config.blue_size;

	if( this->gl_config.alpha_size ) {
		attribs[i++] = EGL_ALPHA_SIZE;
		attribs[i++] = this->gl_config.alpha_size;
	}

	if( this->gl_config.buffer_size ) {
		attribs[i++] = EGL_BUFFER_SIZE;
		attribs[i++] = this->gl_config.buffer_size;
	}

	attribs[i++] = EGL_DEPTH_SIZE;
	attribs[i++] = this->gl_config.depth_size;

	if( this->gl_config.stencil_size ) {
		attribs[i++] = EGL_STENCIL_SIZE;
		attribs[i++] = this->gl_config.stencil_size;
	}

	if( this->gl_config.multisamplebuffers ) {
		attribs[i++] = EGL_SAMPLE_BUFFERS;
		attribs[i++] = this->gl_config.multisamplebuffers;
	}
	
	if( this->gl_config.multisamplesamples ) {
		attribs[i++] = EGL_SAMPLES;
		attribs[i++] = this->gl_config.multisamplesamples;
	}

	attribs[i++] = EGL_NONE;

	if (this->gles_data->eglChooseConfig(this->gles_data->egl_display,
                                             attribs,
                                             &this->gles_data->egl_config, 1,
                                             &found_configs) == EGL_FALSE ||
                                             found_configs == 0) {
		SDL_SetError( "Couldn't find matching EGL config");
		return NULL;
        }

	if (this->gles_data->eglGetConfigAttrib(
	      this->gles_data->egl_display,
	      this->gles_data->egl_config,
	      EGL_NATIVE_VISUAL_ID,
	      (EGLint*)&visual_id) == EGL_FALSE ||
	      !visual_id) {
		/* Use the default visual when all else fails */
		XVisualInfo vi_in;
		int out_count;
		vi_in.screen = SDL_Screen;
		
		this->gles_data->egl_visualinfo = XGetVisualInfo(SDL_Display,
			VisualScreenMask, &vi_in, &out_count);
	}
	else
	{
		XVisualInfo vi_in;
		int out_count;

		vi_in.screen = SDL_Screen;
		vi_in.visualid = visual_id;
		this->gles_data->egl_visualinfo = XGetVisualInfo(SDL_Display,
			VisualScreenMask|VisualIDMask, &vi_in, &out_count);
	}

	return this->gles_data->egl_visualinfo;
#else
	SDL_SetError("X11 driver not configured with OpenGL ES");
	return 0;
#endif
}

int X11_GLES_CreateWindow(_THIS, int w, int h)
{
	int retval;
#ifdef SDL_VIDEO_OPENGL_ES
	XSetWindowAttributes attributes;
	unsigned long mask;
	unsigned long black;
	EGLint depth;
	
#if 0
	/* Get the depth of our config */
	if (this->gles_data->eglGetConfigAttrib(
	      this->gles_data->egl_display,
	      this->gles_data->egl_config,
	      EGL_BUFFER_SIZE, &depth) == EGL_FALSE)
	{
		SDL_SetError("Could not query config depth");
		return -1;
	}

/*
	black = (glx_visualinfo->visual == DefaultVisual(SDL_Display,
						 	SDL_Screen))
	       	? BlackPixel(SDL_Display, SDL_Screen) : 0;
*/
	black = BlackPixel(SDL_Display, SDL_Screen);
	attributes.background_pixel = black;
	attributes.border_pixel = black;
	attributes.colormap = SDL_XColorMap;
	mask = CWBackPixel | CWBorderPixel | CWColormap;

	SDL_Window = XCreateWindow(SDL_Display, WMwindow,
			0, 0, w, h, 0, depth,
			InputOutput, CopyFromParent /*glx_visualinfo->visual*/,
			mask, &attributes);

/*	SDL_Window = XCreateSimpleWindow(SDL_Display, WMwindow,
	                                 0, 0, w, h, 0, 
*/
#endif
	black = (this->gles_data->egl_visualinfo->visual == DefaultVisual(SDL_Display, SDL_Screen))
	       	? BlackPixel(SDL_Display, SDL_Screen) : 0;
	attributes.background_pixel = black;
	attributes.border_pixel = black;
	attributes.colormap = SDL_XColorMap;
	mask = CWBackPixel | CWBorderPixel | CWColormap;

	SDL_Window = XCreateWindow(SDL_Display, WMwindow,
			0, 0, w, h, 0, this->gles_data->egl_visualinfo->depth,
			InputOutput, this->gles_data->egl_visualinfo->visual,
			mask, &attributes);
			
	if ( !SDL_Window ) {
		SDL_SetError("Could not create window");
		return -1;
	}

	/* Create the GLES window surface */
	this->gles_data->egl_surface =
	  this->gles_data->eglCreateWindowSurface(
	    this->gles_data->egl_display,
            this->gles_data->egl_config,
            SDL_Window, NULL);

	if ( this->gles_data->egl_surface == EGL_NO_SURFACE ) {
		SDL_SetError("Could not create GLES window surface");
		return -1;
	}
	
	retval = 0;
#else
	SDL_SetError("X11 driver not configured with OpenGL ES");
	retval = -1;
#endif
	return(retval);
}

int X11_GLES_CreateContext(_THIS)
{
	int retval;
#ifdef SDL_VIDEO_OPENGL_ES
	/* We do this to create a clean separation between X and GLX errors. */
	XSync( SDL_Display, False );
	this->gles_data->egl_context =
	   this->gles_data->eglCreateContext(this->gles_data->egl_display,
	                                     this->gles_data->egl_config,
                                             EGL_NO_CONTEXT, NULL);

	XSync( GFX_Display, False );

	if (this->gles_data->egl_context == EGL_NO_CONTEXT) {
		SDL_SetError("Could not create EGL context");
		return -1;
	}

	this->gles_data->egl_active = 1;
#else
	SDL_SetError("X11 driver not configured with OpenGL ES");
#endif
	if ( this->gles_data->egl_active ) {
		retval = 0;
	} else {
		retval = -1;
	}
	return(retval);
}

void X11_GLES_Shutdown(_THIS)
{
#ifdef SDL_VIDEO_OPENGL_ES
	/* Clean up GLES and EGL */
	if( this->gles_data->egl_context != EGL_NO_CONTEXT ||
	    this->gles_data->egl_surface != EGL_NO_SURFACE ) {
		this->gles_data->eglMakeCurrent(this->gles_data->egl_display, EGL_NO_SURFACE,
		                                EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (this->gles_data->egl_context != EGL_NO_CONTEXT)
		{
			this->gles_data->eglDestroyContext(this->gles_data->egl_display, this->gles_data->egl_context);
			this->gles_data->egl_context = EGL_NO_CONTEXT;
		}

		if (this->gles_data->egl_surface != EGL_NO_SURFACE)
		{
			this->gles_data->eglDestroySurface(this->gles_data->egl_display, this->gles_data->egl_surface);
			this->gles_data->egl_surface = EGL_NO_SURFACE;
		}
	}
	this->gles_data->egl_active = 0;
#endif /* SDL_VIDEO_OPENGL_ES */
}

#ifdef SDL_VIDEO_OPENGL_ES

static int ExtensionSupported(const char *extension)
{
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	/* Extension names should not have spaces. */
	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
	      return 0;
	
	extensions = current_video->GLES_glGetString(GL_EXTENSIONS);
	/* It takes a bit of care to be fool-proof about parsing the
	 *      OpenGL extensions string. Don't be fooled by sub-strings,
	 *           etc. */
	
	start = extensions;
	
	for (;;)
	{
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where) break;
		
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
	        if (*terminator == ' ' || *terminator == '\0') return 1;
						  
		start = terminator;
	}
	
	return 0;
}

/* Make the current context active */
int X11_GLES_MakeCurrent(_THIS)
{
	int retval;
	
	retval = 0;
	if ( ! this->gles_data->eglMakeCurrent(this->gles_data->egl_display,
	                                       this->gles_data->egl_surface,
	                                       this->gles_data->egl_surface,
	                                       this->gles_data->egl_context) ) {
		SDL_SetError("Unable to make EGL context current");
		retval = -1;
	}
	XSync( GFX_Display, False );

	/* More Voodoo X server workarounds... Grr... */
	SDL_Lock_EventThread();
	X11_CheckDGAMouse(this);
	SDL_Unlock_EventThread();

	return(retval);
}

/* Get attribute data from glX. */
int X11_GLES_GetAttribute(_THIS, SDL_GLattr attr, int* value)
{
	EGLint attrib = EGL_NONE;

	switch( attr ) {
	    case SDL_GL_RED_SIZE:
		attrib = EGL_RED_SIZE;
		break;
	    case SDL_GL_GREEN_SIZE:
		attrib = EGL_GREEN_SIZE;
		break;
	    case SDL_GL_BLUE_SIZE:
		attrib = EGL_BLUE_SIZE;
		break;
	    case SDL_GL_ALPHA_SIZE:
		attrib = EGL_ALPHA_SIZE;
		break;
	    case SDL_GL_BUFFER_SIZE:
		attrib = EGL_BUFFER_SIZE;
		break;
	    case SDL_GL_DEPTH_SIZE:
		attrib = EGL_DEPTH_SIZE;
		break;
	    case SDL_GL_STENCIL_SIZE:
		attrib = EGL_STENCIL_SIZE;
		break;
 	    case SDL_GL_MULTISAMPLEBUFFERS:
 		attrib = EGL_SAMPLE_BUFFERS;
 		break;
 	    case SDL_GL_MULTISAMPLESAMPLES:
 		attrib = EGL_SAMPLES;
 		break;
	    default:
		*value = 0;
		return(-1);
	}

	this->gles_data->eglGetConfigAttrib(this->gles_data->egl_display,
	                                    this->gles_data->egl_config,
	                                    attrib, value);

	return 0;
}

void X11_GLES_SwapBuffers(_THIS)
{
	this->gles_data->eglSwapBuffers(this->gles_data->egl_display,
	                                this->gles_data->egl_surface);
}

#endif /* SDL_VIDEO_OPENGL_ES */

void X11_GLES_UnloadLibrary(_THIS)
{
#ifdef SDL_VIDEO_OPENGL_ES
	if ( this->gl_config.driver_loaded ) {
		this->gles_data->eglTerminate(this->gles_data->egl_display);
		
		dlclose(this->gl_config.dll_handle);

		this->gles_data->eglGetProcAddress = NULL;
		this->gles_data->eglChooseConfig = NULL;
		this->gles_data->eglCreateContext = NULL;
		this->gles_data->eglCreateWindowSurface = NULL;
		this->gles_data->eglDestroyContext = NULL;
		this->gles_data->eglDestroySurface = NULL;
		this->gles_data->eglMakeCurrent = NULL;
		this->gles_data->eglSwapBuffers = NULL;
		this->gles_data->eglGetDisplay = NULL;
		this->gles_data->eglTerminate = NULL;

		this->gl_config.dll_handle = NULL;
		this->gl_config.driver_loaded = 0;
	}
#endif
}

#ifdef SDL_VIDEO_OPENGL_ES

/*
 *  A macro for loading a function pointer with dlsym
 */
#define LOAD_FUNC(NAME) \
	*((void**)&this->gles_data->NAME) = dlsym(handle, #NAME); \
	if (!this->gles_data->NAME) \
	{ \
		SDL_SetError("Could not retrieve EGL function " #NAME); \
		return -1; \
	}
  

/* Passing a NULL path means load pointers from the application */
int X11_GLES_LoadLibrary(_THIS, const char* path)
{
	void* handle;
	int dlopen_flags;

 	if ( this->gles_data->egl_active ) {
 		SDL_SetError("OpenGL ES context already created");
 		return -1;
 	}

#ifdef RTLD_GLOBAL
	dlopen_flags = RTLD_LAZY | RTLD_GLOBAL;
#else
	dlopen_flags = RTLD_LAZY;
#endif
	handle = dlopen(path, dlopen_flags);
	/* Catch the case where the application isn't linked with EGL */
	if ( (dlsym(handle, "eglChooseConfig") == NULL) && (path == NULL) ) {
		dlclose(handle);
		path = getenv("SDL_VIDEO_GL_DRIVER");
		if ( path == NULL ) {
			path = DEFAULT_OPENGL;
		}
		handle = dlopen(path, dlopen_flags);
	}
	if ( handle == NULL ) {
		SDL_SetError("Could not load OpenGL ES/EGL library");
		return -1;
	}

	/* Unload the old driver and reset the pointers */
	X11_GLES_UnloadLibrary(this);

	/* Load new function pointers */
	LOAD_FUNC(eglGetDisplay);
	LOAD_FUNC(eglInitialize);
	LOAD_FUNC(eglTerminate);
	LOAD_FUNC(eglGetProcAddress);
	LOAD_FUNC(eglChooseConfig);
	LOAD_FUNC(eglGetConfigAttrib);
	LOAD_FUNC(eglCreateContext);
	LOAD_FUNC(eglDestroyContext);
	LOAD_FUNC(eglCreateWindowSurface);
	LOAD_FUNC(eglDestroySurface);
	LOAD_FUNC(eglMakeCurrent);
	LOAD_FUNC(eglSwapBuffers);

	/*
	 *  Initialize EGL
	 */
	this->gles_data->egl_display = this->gles_data->eglGetDisplay(SDL_Display);

	if ( !this->gles_data->egl_display ) {
		SDL_SetError("Could not get EGL display");
		return -1;
	}

	if ( this->gles_data->eglInitialize(this->gles_data->egl_display, NULL, NULL) != EGL_TRUE ) {
		SDL_SetError("Could not initialize EGL");
		return -1;
	}
	
	this->gl_config.dll_handle = handle;
	this->gl_config.driver_loaded = 1;
	if ( path ) {
		strncpy(this->gl_config.driver_path, path,
			sizeof(this->gl_config.driver_path)-1);
	} else {
		strcpy(this->gl_config.driver_path, "");
	}
	return 0;
}

void *X11_GLES_GetProcAddress(_THIS, const char* proc)
{
	static char procname[1024];
	void* handle;
	void* retval;
	
	handle = this->gl_config.dll_handle;
	if ( this->gles_data->eglGetProcAddress ) {
		retval = this->gles_data->eglGetProcAddress(proc);
		if (retval) {
			return retval;
		}
	}
#if defined(__OpenBSD__) && !defined(__ELF__)
#undef dlsym(x,y);
#endif
	retval = dlsym(handle, proc);
	if (!retval && strlen(proc) <= 1022) {
		procname[0] = '_';
		strcpy(procname + 1, proc);
		retval = dlsym(handle, procname);
	}
	return retval;
}

#endif /* SDL_VIDEO_OPENGL_ES */
