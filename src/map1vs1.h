#pragma once 

#include "map.h"

class Map1vs1 : public Map
{
private:
  /* data */
  int playerId2;
  Camera2D camera2;

public:
  Map1vs1(Level level);
  ~Map1vs1();

  void Draw();

  void Initialize(std::shared_ptr<Map> mapPtr);
  void HandleInput();
  bool IfPlayerDied();
  void MoveCamera();

};
