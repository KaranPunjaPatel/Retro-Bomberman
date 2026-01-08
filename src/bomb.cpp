
#include "bomb.h"
#include "macros.h"
#include <iostream>
#include <algorithm>
#include "asset.h"

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

  DrawTextureEx(texture, Vector2{(float)(BLOCK_SIZE * column),(float)(BLOCK_SIZE * row)}, 0.0f, scale, WHITE);

}

void Bomb::Draw3d()
{ 
  BoundingBox box = GetModelBoundingBox(*model);
  DrawModelEx(*model, {column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, (box.max.y-box.min.y)/2, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2},{0,-1,1}, -30,{.5,.5,.5}, WHITE);
}

bool Bomb::CountDown()
{

    // std::cout << "Here 2 !!!"  << std::endl;
  if(blast.load()) return true;

  float currentTime = GetTime();
  
  if (currentTime - bombTimer > BOMB_TIMER) { // TODO: Blast the bomb
    SetToExplode();

    // std::cout << "BOMB BLAST !!! - " << blast.load() << std::endl;
    return true;
  }

  return false;
}

void Bomb::SetToExplode(){
  bombTimer = 0; 
  blast.store(true);
}

bool Bomb::IsIdPresent(int id){
  
  return std::find(ids.begin(), ids.end(), id) != ids.end();
}

bool Bomb::HasSamePosition(int row, int column)
{
  return (this->row == row && this->column == column);
} 

