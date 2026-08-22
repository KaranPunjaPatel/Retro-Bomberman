
#pragma once

#include <raylib.h>

#include "asset.h"
#include "structure.h"


class Block {
private:
  int row;
  int column;
  float explodeTimer;

public:
  char type;
  Power powerUp;
  float rotation;

  Model *grassModel;

  Texture2D grasstexture;

  Rectangle rect;

  Vector2 texturePart; // Part of texture to draw

  float powerUpRotate;

  Block() = default;
  Block(int row, int column, char type, Power powerUp);
  Block(int row, int column, char type, Power powerUp, Model *model);

  void Draw();
  void Draw3d();

  bool IsBlock();
  bool IsBrick();
  bool IsClay();
  void SetType(char type);
  void SetPowerType(Power power);

  void SetExplodeTimer();
  bool ClearExplode();
  bool CollidingWithPowerUp(Rectangle rect);
};
