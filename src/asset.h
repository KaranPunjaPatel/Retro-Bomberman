#pragma once
#ifndef ASSET_H
#define ASSET_H

#include <raylib.h>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include "macros.h"
#include "raymath.h"

// Define asset type constants
#define t_GRASS1      0
#define t_GRASS2      1
#define t_GRASS3      2
#define t_TITLE       3
#define t_BRICK       4
#define t_CLAY        5
#define t_PLAYER      6
#define t_ENEMY       7
#define t_BOMB        8
#define t_EXPLOSION   9
#define t_SPEED       10
#define t_BOMB_RADIUS 11
#define t_BOMB_COUNT  12
#define t_BG          13
#define t_HEART       14
#define t_RADIUS      15
#define t_COUNT       16
#define t_CLOCK       17

#define m_PLAYER      0
#define m_EMEMY       1
#define m_BOX         2
#define m_BOUNDARY    3
#define m_BRICK       4
#define m_BOMB        5
#define m_SPEED       6
#define m_BOMB_RADIUS 7
#define m_BOMB_COUNT  8


#define END         Vector2{.x = 64, .y = 0 }
#define PIPE        Vector2{.x = 64, .y = 32}
#define MIDDLE      Vector2{.x = 32, .y = 32}
#define FOUR_MERGE  Vector2{.x =  0, .y = 32}
#define THREE_MERGE Vector2{.x = 32, .y = 0 }
#define TWO_MERGE   Vector2{.x =  0, .y = 0 }

// Declare the textures vector as extern
// extern std::vector<Texture2D> textures;

// Function declarations
void loadTextures();
Texture2D getTexture(int asset);

void unloadAssets();

void loadModels();
void pushModel(const char* file, float width, float length);
std::shared_ptr<Model> getModel(int asset);

void loadAnimations();
std::pair<ModelAnimation*,int> getAnimation(int asset);

void loadAssets();

// extern std::atomic_bool b_textures   ; 
// extern std::atomic_bool b_models     ; 
// extern std::atomic_bool b_animations ; 

// std::shared_ptr<Model> CloneModel(const std::shared_ptr<Model>& originalModel);

#endif // ASSET_H
