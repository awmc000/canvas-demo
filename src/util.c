#include "math.h"
#include "util.h"

// llm shlop
void
drawDottedLine(int startX, int startY, int endX, int endY, Color color,
  int segmentSize)
{
    // Calculate the full line length
    float dx     = endX - startX;
    float dy     = endY - startY;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize direction vector
    float directionX = dx / length;
    float directionY = dy / length;

    // Step through the line, segment on, segment off
    for (float pos = 0; pos < length; pos += segmentSize * 2) {
        float x1 = startX + directionX * pos;
        float y1 = startY + directionY * pos;

        float segmentEnd = pos + segmentSize;
        if (segmentEnd > length) segmentEnd = length;

        float x2 = startX + directionX * segmentEnd;
        float y2 = startY + directionY * segmentEnd;

        DrawLine((int) x1, (int) y1, (int) x2, (int) y2, color);
    }
}
