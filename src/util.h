#pragma once
#include <string>
#include <raylib.h>
#include <cmath>

bool Vector2Equal(Vector2 first, Vector2 second);

void getTime(double timeInSeconds, char timeStr[6]);

float normalizeAngle(float angle);