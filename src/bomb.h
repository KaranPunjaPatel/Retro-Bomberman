#pragma once

#include <atomic>
#include <vector>
#include <memory>

#include <raylib.h>

class Bomb
{
private:
  Bomb(const Bomb &) = delete;
  Bomb &operator=(const Bomb &) = delete;

protected:
  Texture2D texture;

  int owner;
  int row;
  int column;

  float bombTimer;
  std::atomic_bool blast;

  size_t bombRadius;

  std::vector<int> ids; // The id of player already interacting with the bomb
  std::shared_ptr<Model> model;

public:
  Bomb(int id, int row, int column, size_t bombRadius, std::vector<int> ids);
  ~Bomb();

  void Draw();
  void Draw3d();

  [[nodiscard]] bool CountDown() noexcept;
  [[nodiscard]] bool IsIdPresent(int id) const noexcept;
  void SetToExplode() noexcept;

  [[nodiscard]] bool HasExploded() const noexcept;
  [[nodiscard]] bool HasSamePosition(int row, int column) const noexcept;
  [[nodiscard]] int GetOwner() const noexcept;
  [[nodiscard]] int GetRow() const noexcept;
  [[nodiscard]] int GetColumn() const noexcept;
  [[nodiscard]] size_t GetBombRadius() const noexcept;
  [[nodiscard]] std::vector<int> GetIds() const noexcept;
  [[nodiscard]] std::shared_ptr<Model> GetModel() const noexcept;

  void BombIdsRemove(int id) noexcept;
};
