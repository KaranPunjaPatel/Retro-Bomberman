
#pragma once 

#include <raylib.h>
#include <memory>

#include "human.h"

class Player : public Human
{
private:
  int rotateSpeed = 10;

public:
  Player(std::shared_ptr<Map> map, int id, size_t row, size_t column);
  ~Player();

  int lifeCount;

  void HandleInput();
  void HandleInput2();
  void Draw();
  void Draw3d();

  virtual void Dies();

  // Texture2D grass;

  int animCount = 0;
	int animFrame = 0;
  int rotationAngle = 180;

	std::shared_ptr<Model> model;
	ModelAnimation* animation;
};

