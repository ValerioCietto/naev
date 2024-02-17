/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>

/* Include header generated by glad */
#include "glad.h"

#include "SDL.h"
/** @endcond */

#include "colour.h"
/* We put all the other opengl stuff here to only have to include one header.  */
#include "opengl_render.h"
#include "opengl_shader.h"
#include "opengl_tex.h"
#include "opengl_vbo.h"
#include "mat4.h"
#include "shaders.gen.h"

#define OPENGL_NUM_FBOS    4 /**< Number of FBOs to allocate and deal with. */
/** Currently used FBO IDs:
 * 0/1: front/back buffer for rendering
 * 2: temporary scratch buffer to use as necessary
 * 3: Used by toolkit */

/*
 * Contains info about the opengl screen
 */
#define OPENGL_DOUBLEBUF   (1<<1) /**< Doublebuffer. */
#define OPENGL_VSYNC       (1<<2) /**< Sync to monitor vertical refresh rate. */
#define OPENGL_SUBROUTINES (1<<3) /**< Ability to use shader subroutines. */
#define gl_has(f)    (gl_screen.flags & (f)) /**< Check for the flag */
/**
 * @brief Stores data about the current opengl environment.
 */
typedef struct glInfo_ {
   int major;  /**< OpenGL major version. */
   int minor;  /**< OpenGL minor version. */
   int glsl;   /**< GLSL version. */
   int x; /**< X offset of window viewport. */
   int y; /**< Y offset of window viewport. */
   /* Viewport considers x/y offset. */
   int w; /**< Window viewport width. */
   int h; /**< Window viewport height. */
   /* Scaled window is the effective window resolution without considering offsets. */
   int nw; /**< Scaled window width. */
   int nh; /**< Scaled window height. */
   /* Real window resolution is the real window resolution, unscaled and without offsets. */
   int rw; /**< Real window width. */
   int rh; /**< Real window height. */
   double scale; /**< Scale factor. */
   double wscale; /**< Width scale factor. */
   double hscale; /**< Height scale factor. */
   double dwscale; /**< Drawable height scale factor. */
   double dhscale; /**< Drawable width scale factor. */
   double mxscale; /**< Mouse X scale factor. */
   double myscale; /**< Mouse y scale factor. */
   int depth; /**< Depth in bpp */
   int r; /**< How many red bits we have. */
   int g; /**< How many green bits we have. */
   int b; /**< How many blue bits we have. */
   int a; /**< How many alpha bits we have. */
   unsigned int flags; /**< Stores different properties */
   int tex_max; /**< Maximum texture size */
   int multitex_max; /**< Maximum multitexture levels */
   int fsaa; /**< Full Scene Anti Aliasing level. */
   SDL_Window *window; /**< Window for SDL2. */
   SDL_GLContext context; /**< Context for OpenGL. */
   GLuint current_fbo; /**< Current framebuffer. */
   GLuint fbo[OPENGL_NUM_FBOS]; /**< Framebuffers. */
   GLuint fbo_tex[OPENGL_NUM_FBOS]; /**< Texture for framebuffers. */
   GLuint fbo_depth_tex[OPENGL_NUM_FBOS]; /**< Depth texture for framebuffers. */
} glInfo;
extern glInfo gl_screen; /* local structure set with gl_init and co */

extern mat4 gl_view_matrix;

#define  SCREEN_X gl_screen.x /**< Screen X offset. */
#define  SCREEN_Y gl_screen.y /**< Screen Y offset. */
#define  SCREEN_W gl_screen.w /**< Screen width. */
#define  SCREEN_H gl_screen.h /**< Screen height. */

/*
 * used with colour.h
 */
#define COLOUR(x)    glColour4d((x).r,(x).g,(x).b,(x).a) /**< Change colour. */
#define ACOLOUR(x,a) glColour4d((x).r,(x).g,(x).b,a) /**< Change colour and override alpha. */

/*
 * initialization / cleanup
 */
int gl_init (void);
void gl_exit (void);
void gl_resize (void);

/*
 * Extensions and version.
 */
GLboolean gl_hasVersion( int major, int minor );

/*
 * Viewport.
 */
void gl_windowToScreenPos( int *sx, int *sy, int wx, int wy );
void gl_screenToWindowPos( int *wx, int *wy, int sx, int sy );
void gl_viewport( int x, int y, int w, int h );
void gl_defViewport (void);
void gl_setDefViewport( int x, int y, int w, int h );
int gl_setupFullscreen (void);

/*
 * misc
 */
void gl_colourblind (void);
GLint gl_stringToFilter( const char *s );
GLint gl_stringToClamp( const char *s );
void gl_screenshot( const char *filename );
#ifdef DEBUGGING
#define gl_checkErr()   gl_checkHandleError( __func__, __LINE__ )
void gl_checkHandleError( const char *func, int line );
#else /* DEBUGGING */
#define gl_checkErr() /**< Hack to ignore errors when debugging. */
#endif /* DEBUGGING */
