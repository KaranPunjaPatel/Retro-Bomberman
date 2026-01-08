#pragma once 

#include <raylib.h>
#include <atomic>
#include <vector>
#include <memory>

class Bomb
{
private:
  Bomb(const Bomb&) = delete;
  Bomb& operator=(const Bomb&) = delete;

public:
  Texture2D texture;

  int owner;
  int row;
  int column;

  
  float bombTimer;
  std::atomic_bool blast;

  size_t bombRadius;

  std::vector<int> ids; // The id of player already interacting with the bomb

  Bomb(int id, int row, int column, size_t bombRadius, std::vector<int> ids);
  ~Bomb();

  void Draw();
  void Draw3d();

  bool CountDown();
  bool IsIdPresent(int id);
  bool HasSamePosition(int row, int column);
  void SetToExplode();

  std::shared_ptr<Model> model;

};

