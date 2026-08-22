
#include <iostream>
#include <algorithm>

#include "asset.h"
#include "bomb.h"
#include "macros.h"

Bomb::Bomb(int id, int row, int column, size_t bombRadius, std::vector<int> ids)
    : owner(id),
      row(row),
      column(column),
      blast(false),
      bombRadius(bombRadius),
      ids(ids)
{
  bombTimer = GetTime();

  texture = getTexture(t_BOMB);

  model = getModel(m_BOMB);
}

Bomb::~Bomb()
{
}

void Bomb::Draw()
{
  // DrawCircle(BLOCK_SIZE * column + BLOCK_SIZE/2, BLOCK_SIZE * row + BLOCK_SIZE/2, BLOCK_SIZE/2.5, BLACK);
  float scale = std::min(BLOCK_SIZE / texture.width, BLOCK_SIZE / texture.height);

  DrawTextureEx(texture, Vector2{(float)(BLOCK_SIZE * column), (float)(BLOCK_SIZE * row)}, 0.0f, scale, WHITE);
}

void Bomb::Draw3d()
{
  BoundingBox box = GetModelBoundingBox(*model);
  DrawModelEx(*model, {column * BLOCK_SIZE_3D + BLOCK_SIZE_3D / 2, (box.max.y - box.min.y) / 2, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D / 2}, {0, -1, 1}, -30, {.5, .5, .5}, WHITE);
}

[[nodiscard]] bool Bomb::CountDown() noexcept
{
  if (blast.load())
    return true;

  float currentTime = GetTime();

  if (currentTime - bombTimer > BOMB_TIMER)
  {
    SetToExplode();
    return true;
  }

  return false;
}

void Bomb::SetToExplode() noexcept
{
  bombTimer = 0;
  blast.store(true);
}

[[nodiscard]] bool Bomb::IsIdPresent(int id) const noexcept
{
  return std::find(ids.begin(), ids.end(), id) != ids.end();
}

[[nodiscard]] bool Bomb::HasSamePosition(int row, int column) const noexcept { return (this->row == row && this->column == column); }

[[nodiscard]] bool Bomb::HasExploded() const noexcept { return blast.load(); }

[[nodiscard]] int Bomb::GetOwner() const noexcept { return owner; }
[[nodiscard]] int Bomb::GetRow() const noexcept { return row; }
[[nodiscard]] int Bomb::GetColumn() const noexcept { return column; }
[[nodiscard]] size_t Bomb::GetBombRadius() const noexcept { return bombRadius; }
[[nodiscard]] std::vector<int> Bomb::GetIds() const noexcept { return ids; }
[[nodiscard]] std::shared_ptr<Model> Bomb::GetModel() const noexcept { return model; }

void Bomb::BombIdsRemove(int id) noexcept
{ // Removes ids of bombs which are not needed for explosion
  ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
}
