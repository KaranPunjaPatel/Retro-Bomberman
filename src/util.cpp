#include "util.h"

bool Vector2Equal(Vector2 first, Vector2 second){
  return (first.x == second.x && first.y == second.y);
}

void getTime(double timeInSeconds, char timeStr[6]) {
  int minutes = static_cast<int>(timeInSeconds) / 60;
  int seconds = static_cast<int>(timeInSeconds) % 60;

  sprintf(timeStr, "%02d:%02d", minutes, seconds); // "MM:SS\0"
}

float normalizeAngle(float angle) {
    angle = (float)std::fmod(angle, 360.0);
    if (angle > 180.0)
        angle -= 360.0;
    else if (angle <= -180.0)
        angle += 360.0;
    return angle;
}