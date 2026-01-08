#pragma once
#ifndef STRUCTURE_H
#define STRUCTURE_H
#include <cstdint>
#include <vector>
#include <raylib.h>
#include <iostream>

#include "macros.h"
// #include "player.h"
// class Enemy;

struct PowerUp
{
  size_t row;
  size_t column;
  Power type;
};

struct Position
{
  size_t row;
  size_t column;
  int type;
  Direction direction;
};


// Might also need a playerCount as well like uint8_t playerCount

struct Level
{
  LvlType type;
  size_t rows;
  size_t columns;
  
  std::vector<std::vector<char>> map;
  std::vector<PowerUp> powerUps;
  std::vector<Position> enemies;
  std::vector<Position> players;

  float  speed;
  size_t bombCount;
  size_t bombRadius;

};




#endif