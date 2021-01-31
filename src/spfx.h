/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPFX_H
#  define SPFX_H


#include "ntime.h"
#include "opengl.h"
#include "physics.h"


#define SPFX_LAYER_FRONT   0 /**< Front spfx layer. */
#define SPFX_LAYER_BACK    1 /**< Back spfx layer. */

#define SHAKE_DECAY        0.3 /**< Rumble decay parameter */
#define SHAKE_MAX          1.0 /**< Rumblemax parameter */


/**
 * @brief represents a set of colour for trails.
 */
typedef struct trailColour_ {
   char* name;       /**< Colour set's name. */
   glColour idle_col; /**< Colour when idle. */
   glColour glow_col; /**< Colour when thrusting. */
   glColour aftb_col; /**< Colour when afterburning. */
   glColour jmpn_col; /**< Colour when jumping. */
} trailColour;


typedef struct trailPoint_ {
   Vector2d p;    /**< Control points for the trail. */
   glColour c;   /**< Colour associated with the trail's control points. */
   double t; /**< Timer, normalized to the time to live of the trail (starts at 1, ends at 0). */
} trailPoint;


/**
 * @struct Trail_spfx
 *
 * @brief A trail generated by a ship or an ammo.
 */
typedef struct Trail_spfx_ {
   trailPoint *points; /**< Trail points. */
} Trail_spfx;


/*
 * Array of trail colours
 */
extern trailColour* trail_col_stack;


/*
 * stack manipulation
 */
int spfx_get( char* name );
int trailType_get( char* name );
void spfx_add( const int effect,
      const double px, const double py,
      const double vx, const double vy,
      const int layer );


/*
 * stack mass manipulation functions
 */
void spfx_update( const double dt );
void spfx_render( const int layer );
void spfx_clear (void);
void spfx_trail_create( Trail_spfx* );
unsigned int spfx_trail_update( Trail_spfx* trail, double dt );
void spfx_trail_grow( Trail_spfx* trail, Vector2d pos, glColour col  );
void spfx_trail_remove( Trail_spfx* trail );


/*
 * get ready to rumble
 */
void spfx_begin( const double dt, const double real_dt );
void spfx_end (void);
void spfx_shake( double mod );
void spfx_getShake( double *x, double *y );


/*
 * other effects
 */
void spfx_cinematic (void);


/*
 * spfx effect loading and freeing
 */
int spfx_load (void);
void spfx_free (void);


#endif /* SPFX_H */
