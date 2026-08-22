#pragma once

#include <cstdint>

constexpr int ROW = 11;
constexpr int COL = 17;
constexpr float BLOCK_SIZE = 60.0f;
constexpr float BLOCK_SIZE_3D = 1.0f;
constexpr float BLOCK_HEIGHT_3D = 1.0f;

constexpr int PLAYER = 94;      // ^
constexpr int GRASS_BLOCK = 46; // .
constexpr int BRICK_BLOCK = 35; // #
constexpr int CLAY_BLOCK = 33;  // !
constexpr int BOMB = 38;        // &
constexpr int EXPLODE = 63;     // ?

constexpr int BOMB_TIMER = 3;
constexpr int EXPLODE_TIMER = 2;
constexpr int SHOW_2D = 0;
constexpr float ROTATION_SPEED = 5.0f;
constexpr float BLOCK_HITBOX_PADDING = (BLOCK_SIZE * 0.1f);

// Change here for starting specs
constexpr float DEF_SPEED = 180.0f;
constexpr int DEF_BOMBS = 1;
constexpr int DEF_BOMB_RADIUS = 1;

enum struct Power : uint8_t { NO_POWER, SPEED, BOMB_RADIUS, BOMB_COUNT };

enum struct Direction : uint8_t {
  NORTH, // 0
  EAST,  // 1
  SOUTH, // 2
  WEST,  // 3
  CENTER // 4
};

enum struct LvlType : uint8_t {
  OFFLINE,    // 0
  ONE_VS_ONE, // 1
  ONLINE,     // 2
};
