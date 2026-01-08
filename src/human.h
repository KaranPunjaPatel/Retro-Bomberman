#pragma once
#include <raylib.h>

#include <vector> 
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <cmath>

#include "bomb.h"
#include "macros.h"
#include "util.h"

class Map;

class Human {

// private:
public:
  Human(std::shared_ptr<Map> map, int id, size_t row, size_t column);
  ~Human();

  std::shared_ptr<Map> map;
  
  float  speed;
  size_t bombCount;
  size_t bombRadius;

  bool alive;

  int id;
  
  Rectangle rect;

  int rowPos;
  int colPos;

  Direction direction; 

  Texture2D texture;

  virtual void Dies() = 0;

  void Draw();
  void Draw3d();

  void ChangePosition();
  void IsTouchingBoundary();

  void SetDirection(Direction direction);



};
