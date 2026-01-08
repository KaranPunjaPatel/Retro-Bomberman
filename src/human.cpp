
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
        alive(true),
        id(id), 
        rowPos(row),
        colPos(column)
{
  ChangePosition();
  
}

Human::~Human(){}

void Human::Draw()
{
  DrawRectangleRec(rect, WHITE);
}

void Human::Draw3d()
{
  
}


void Human::ChangePosition()
{
  int centerX = rect.x + rect.width/2;
  int centerY = rect.y + rect.height/2;

  int row    = (centerY - (centerY % (int)BLOCK_SIZE))/BLOCK_SIZE;
  int column = (centerX - (centerX % (int)BLOCK_SIZE))/BLOCK_SIZE;

  rowPos = row;
  colPos = column;
}

void Human::IsTouchingBoundary()
{

  if(rect.x < 0) rect.x = 0;
  else if(rect.x + rect.width > map->mapWidth) rect.x = map->mapWidth - rect.width;
  else if(rect.y < 0) rect.y = 0;
  else if(rect.y + rect.height > map->mapHeight) rect.y = map->mapHeight - rect.height;
}

void Human::SetDirection(Direction direction)
{
  this->direction = direction;
}


