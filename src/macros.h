
#pragma once
#include <cstdint>
#define ROW         11
#define COL         17
#define BLOCK_SIZE  60.0f
#define BLOCK_SIZE_3D     1.0f
#define BLOCK_HEIGHT_3D   1.0f

#define PLAYER      94  // ^
#define GRASS_BLOCK 46  // .
#define BRICK_BLOCK 35  // #
#define CLAY_BLOCK  33  // !
#define BOMB        38  // &
#define EXPLODE     63  // ?

#define BOMB_TIMER      3
#define EXPLODE_TIMER   2
#define SHOW_2D 0
#define ROTATION_SPEED  5.0f


// Change here for starting specs
#define DEF_SPEED       180.0f
#define DEF_BOMBS       1
#define DEF_BOMB_RADIUS 1

enum struct Power : uint8_t {
  NO_POWER,
  SPEED,
  BOMB_RADIUS,
  BOMB_COUNT
};

enum struct Direction : uint8_t {
  NORTH,  // 0
  EAST,   // 1
  SOUTH,  // 2
  WEST,   // 3
  CENTER  // 4
};

enum struct LvlType : uint8_t {
  OFFLINE,    // 0
  ONE_VS_ONE, // 1
  ONLINE,     // 2
};

