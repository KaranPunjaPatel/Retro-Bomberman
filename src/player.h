
#pragma once

#include <memory>
#include <vector>

#include <raylib.h>

#include "human.h"

constexpr int PLAYER_ONE = 0;
constexpr int PLAYER_TWO = 1;

class Player : public Human
{
private:
  int rotateSpeed = 10;

public:
  Player(std::shared_ptr<Map> map, int id, size_t row, size_t column, int player = PLAYER_ONE);
  ~Player();

  void HandleInput();
  void HandleInput2();
  void Draw();
  void Draw3d();

  virtual void Dies();

  int player = 0;
  int lifeCount = 2;

  size_t startRow = 0;
  size_t startCol = 0;

  int animCount = 0;
  int animFrame = 0;
  int rotationAngle = 180;

  std::shared_ptr<Model> model;
  ModelAnimation *animation;
};
