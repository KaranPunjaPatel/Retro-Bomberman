#pragma once

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "macros.h"

// Define asset type constants
constexpr int t_GRASS1 = 0;
constexpr int t_GRASS2 = 1;
constexpr int t_GRASS3 = 2;
constexpr int t_TITLE = 3;
constexpr int t_BRICK = 4;
constexpr int t_CLAY = 5;
constexpr int t_PLAYER = 6;
constexpr int t_ENEMY = 7;
constexpr int t_BOMB = 8;
constexpr int t_EXPLOSION = 9;
constexpr int t_SPEED = 10;
constexpr int t_BOMB_RADIUS = 11;
constexpr int t_BOMB_COUNT = 12;
constexpr int t_BG = 13;
constexpr int t_HEART = 14;
constexpr int t_RADIUS = 15;
constexpr int t_COUNT = 16;
constexpr int t_CLOCK = 17;

constexpr int m_PLAYER = 0;
constexpr int m_ENEMY = 1;
constexpr int m_BOX = 2;
constexpr int m_BOUNDARY = 3;
constexpr int m_BRICK = 4;
constexpr int m_BOMB = 5;
constexpr int m_SPEED = 6;
constexpr int m_BOMB_RADIUS = 7;
constexpr int m_BOMB_COUNT = 8;

constexpr Vector2 END = Vector2{.x = 64, .y = 0};
constexpr Vector2 PIPE = Vector2{.x = 64, .y = 32};
constexpr Vector2 MIDDLE = Vector2{.x = 32, .y = 32};
constexpr Vector2 FOUR_MERGE = Vector2{.x = 0, .y = 32};
constexpr Vector2 THREE_MERGE = Vector2{.x = 32, .y = 0};
constexpr Vector2 TWO_MERGE = Vector2{.x = 0, .y = 0};

// Declare the textures vector as extern
// extern std::vector<Texture2D> textures;

// Function declarations
void loadTextures();
Texture2D getTexture(int asset);

void unloadAssets();

void loadModels();
void pushModel(const char *file, float width, float length);
std::shared_ptr<Model> getModel(int asset);

void loadAnimations();
std::pair<ModelAnimation *, int> getAnimation(int asset);

void loadAssets();

// extern std::atomic_bool b_textures   ;
// extern std::atomic_bool b_models     ;
// extern std::atomic_bool b_animations ;

// std::shared_ptr<Model> CloneModel(const std::shared_ptr<Model>&
// originalModel);
