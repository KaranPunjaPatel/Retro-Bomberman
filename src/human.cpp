
#include <iostream>
#include <limits>

#include "human.h"
#include "map.h"
#include "macros.h"

Human::Human(std::shared_ptr<Map> map, int id, size_t row, size_t column)
    : map(map),
      speed(DEF_SPEED),
      bombCount(DEF_BOMBS),
      bombRadius(DEF_BOMB_RADIUS),
      id(id),
      rowPos(row),
      colPos(column),
      alive(true)
{
  ChangePosition();
}

Human::~Human() {}

void Human::Draw()
{
  DrawRectangleRec(rect, WHITE);
}

void Human::Draw3d()
{
}

[[nodiscard]] bool Human::IsAlive() const noexcept { return this->alive; }
[[nodiscard]] int Human::GetRowPos() const noexcept { return this->rowPos; }
[[nodiscard]] int Human::GetColumnPos() const noexcept { return this->colPos; }
[[nodiscard]] Rectangle &Human::GetRectangle() noexcept { return this->rect; }
[[nodiscard]] size_t Human::GetBombCount() const noexcept { return this->bombCount; }
[[nodiscard]] size_t Human::GetBombRadius() const noexcept { return this->bombRadius; }
[[nodiscard]] float Human::GetSpeed() const noexcept { return this->speed; }
[[nodiscard]] Direction Human::GetDirection() const noexcept { return this->direction; }

void Human::ChangePosition()
{
  int centerX = (int)(rect.x + rect.width / 2.0f);
  int centerY = (int)(rect.y + rect.height / 2.0f);

  int row = centerY / (int)BLOCK_SIZE;
  int column = centerX / (int)BLOCK_SIZE;

  if (row < 0)
    row = 0;
  if (column < 0)
    column = 0;

  this->rowPos = (size_t)row;
  this->colPos = (size_t)column;
}

void Human::IsTouchingBoundary()
{
  if (rect.x < 0)
    rect.x = 0;
  else if (rect.x + rect.width > map->mapWidth)
    rect.x = map->mapWidth - rect.width;
  else if (rect.y < 0)
    rect.y = 0;
  else if (rect.y + rect.height > map->mapHeight)
    rect.y = map->mapHeight - rect.height;
}

void Human::SetDirection(Direction newDirection) noexcept { this->direction = newDirection; }
void Human::SetSpeed(float amount) noexcept { this->speed = amount; }
void Human::SetBombCount(size_t amount) noexcept { this->bombCount = amount; }
void Human::SetBombRadius(size_t amount) noexcept { this->bombRadius = amount; }

void Human::ModifyStat(StatType stat, float amount) noexcept
{
  switch (stat)
  {
  case StatType::Speed:
    this->speed *= amount;
    break;
  case StatType::BombRadius:
    this->bombRadius += static_cast<int>(amount);
    break;
  case StatType::BombCount:
    this->bombCount += static_cast<int>(amount);
    break;
  }
}
