/**
 * utilities and shortcuts to go on top of Raylib
 */
#ifndef UTIL_H
#define UTIL_H

#include "raylib.h"

// RLAPI void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);

/**
 * Wrapper for DrawLine which draws the line in segments of the given size,
 * which should be at least 1.
 */
void
drawDottedLine(int startX, int startY, int endX, int endY, Color color,
  int segmentSize);

#endif // UTIL_H
