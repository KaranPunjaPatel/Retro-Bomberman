#pragma once 

#include <raylib.h>
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

#include "structure.h"

#include "player.h"
#include "enemy.h"

#include "macros.h"
#include "block.h"

#define MAP_BORDER 10.0f
#define UI_OFFSET  10.0f
#define UI_TEXTURE_SIZE  32.0f
#define UI_FONT_SIZE  36
#define UI_MARGIN  50

class Map 
{
public:
  Map(Level level);
  ~Map();

  /* Game Calls */
  virtual void Initialize(std::shared_ptr<Map> mapPtr);
  void Print();
  virtual void Draw();
  
protected:
  /* Thread calls */
  void StartBombCountdownThread();
  
public:
  bool WithinBoundary(size_t row, size_t column); // checking the boundry

  bool IsColliding(int id);

  void ChangeBombIds(int id);
  virtual void HandleInput();
  bool AlreadyHasBomb(size_t row, size_t column);

  void AddBomb(int id);

  void ExplodeBomb();
  void ExplodeArea(std::shared_ptr<Bomb> bomb);
  int ExplodeAreaHelper(size_t row, size_t column);
  
  bool IsInExplosion(int id);

  bool IsEnemyColliding(int id);
  bool IsEnemyCollidingHelper(size_t row, size_t column);
  
  bool CollidingWithEnemy(int id);
  void CollidingWithPowerUp(int id);
  void RedrawExplodeArea();
  int CheckIfExplosion(size_t row, size_t column);
  
  virtual void MoveCamera();
  void DrawUI();
  void DrawLevelEndUI(const char* text);

  void DrawMapBoundary(float width, float length);


  bool AllEnemiesDied();
  virtual bool IfPlayerDied();

  
  std::vector<std::vector<Block>> map;
  std::unordered_map<int, std::shared_ptr<Player>> players;
  std::unordered_map<int,std::shared_ptr<Enemy>> enemies;

  int mapWidth;
  int mapHeight;
  double startTime;
  double finishTime;

  Level level;

  size_t numRows;
  size_t numCols;
  int cellSize;

  int playerId;

  std::atomic_bool stopFlag;    // To stop the bomb timer thread
  std::atomic_bool explodeBomb; // Check this to see if any bomb exploded
  
  Camera2D camera;
  Camera3D camera3d;

  std::vector<std::shared_ptr<Bomb>> bombsArr;
  std::mutex bombsMutex;        // Mutex to access bomb vector safely

  std::thread bombCountdown;    // Thread to count bomb timer

  std::atomic_bool clearExplode; // Check this to see if we need to clean exploded area
  std::vector<std::pair<int,int>> explodeArr; 
  std::vector<std::shared_ptr<Bomb>> processedBombsArr;

  Model grass[3];
  std::shared_ptr<Model> boundaryModel;
};