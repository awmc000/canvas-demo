/**
 * The plane is generally larger than the screen, and the viewport is the rectangle
 * currently being viewed in the window.
 */
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "raylib.h"
#define BORDER_MARGIN 10

/*****************************************************************************
* Structs and Typedefs
*****************************************************************************/

struct viewport {
    int   y;
    int   x;
    int   w;
    int   h;
    float scale; // TODO: implement zoom, which will require scaling all drawn objects
};

/*****************************************************************************
* Functions Provided
*****************************************************************************/

/**
 * Converts a Position Y coordinate to a screen Y coordinate.
 */
int projectX(struct viewport * vp, int positionX);

/**
 * Converts a Position Y coordinate to a screen Y coordinate
 * clamping coordinates if desired, else placing them far offscreen.
 */
int clampProjectX(struct viewport * vp, int positionX, int clamp);

/**
 * Converts a Position X coordinate to a screen X coordinate.
 */
int projectY(struct viewport * vp, int positionY);

/**
 * Converts a Position X coordinate to a screen X coordinate.
 * clamping coordinates if desired, else placing them far offscreen.
 */
int clampProjectY(struct viewport * vp, int positionY, int clamp);

/**
 * Returns 1 if the given world position is visible
 * in the current viewport.
 */
int positionVisible(struct viewport * vp, int y, int x);

#endif // VIEWPORT_H
