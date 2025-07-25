#include "viewport.h"

/**
 * Converts a Position Y coordinate to a screen Y coordinate.
 */
int
clampProjectY(struct viewport * vp, int positionY, int clamp)
{
    // Case: positionY is above the screen => clamp to top border
    if (positionY < vp->y)
        return clamp ? vp->h - BORDER_MARGIN : -1000;
    // Case: positionY is below the screen => clamp to bottom border
    else if (positionY > vp->y + vp->h)
        return clamp ? BORDER_MARGIN : -1000;
    // Case: positionY is on screen
    else
        return vp->h - (positionY - vp->y);
}

/**
 * Converts a Position X coordinate to a screen X coordinate.
 */
int
clampProjectX(struct viewport * vp, int positionX, int clamp)
{
    // Case: positionX is left of the screen => clamp to left border
    if (positionX < vp->x)
        return clamp ? BORDER_MARGIN : -1000;
    // Case: positionX is right of the screen => clamp to right border
    else if (positionX > vp->x + vp->w)
        return clamp ? vp->w - BORDER_MARGIN : -1000;
    // Case: positionX is on screen
    else
        return positionX - vp->x;
}

/**
 * Returns 1 if the given world position is visible
 * in the current viewport.
 */
int
positionVisible(struct viewport * vp, int y, int x)
{
    int yInRange = (y >= 0 + vp->y) && (y <= (vp->y + vp->h));
    int xInRange = (x >= 0 + vp->x) && (x <= (vp->x + vp->w));

    return yInRange && xInRange;
}
