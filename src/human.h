#pragma once

#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <raylib.h>

#include "bomb.h"
#include "macros.h"
#include "util.h"

class Map;

enum class StatType
{
  Speed,
  BombRadius,
  BombCount
};

class Human
{
public:
  Human(std::shared_ptr<Map> map, int id, size_t row, size_t column);
  ~Human();

  [[nodiscard]] Rectangle &GetRectangle() noexcept;
  [[nodiscard]] int GetRowPos() const noexcept;
  [[nodiscard]] int GetColumnPos() const noexcept;
  [[nodiscard]] bool IsAlive() const noexcept;
  [[nodiscard]] size_t GetBombCount() const noexcept;
  [[nodiscard]] size_t GetBombRadius() const noexcept;
  [[nodiscard]] float GetSpeed() const noexcept;
  [[nodiscard]] Direction GetDirection() const noexcept;

  virtual void Dies() = 0;

  void Draw();
  void Draw3d();

  void ChangePosition();
  void IsTouchingBoundary();

  void SetDirection(Direction newDirection) noexcept;
  void ModifyStat(StatType stat, float amount) noexcept;
  void SetSpeed(float amount) noexcept;
  void SetBombCount(size_t amount) noexcept;
  void SetBombRadius(size_t amount) noexcept;

protected:
  std::shared_ptr<Map> map;

  float speed;
  size_t bombCount;
  size_t bombRadius;

  int id;
  int rowPos;
  int colPos;

  bool alive;
  Direction direction;

  Rectangle rect;
  Texture2D texture;
};
