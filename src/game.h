#pragma once

#include <raylib.h> 
#include <vector> 
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>

#include "structure.h"

#include "map1vs1.h"
#include "bomb.h"

#include "button.h"

class Game
{
private:

  int prevScreenWidth;  
  int prevScreenHeight;  
  
  std::shared_ptr<Map> map;

  Texture2D textureTitle;

  int page;
  Button startButton;
  Button oneVsOneButton;
  Button onlineButton;

  Button returnButton;
  
  std::vector<LevelButton> lvlBtnArr;

  float bgOffsetX;    // Horizontal offset for movement
  float speed;        // Speed in pixels per second
  Texture2D background;


public:
  Game();
  ~Game();
  
  void MoveBg();
  void DrawBg();

  void HandleChanges(bool definitely);
  
  void Draw();
  void HandleInput();
  
  void DrawStartingPage();
  void HandleStartingInput();

  void LoadLevelChoosingPage();
  void DrawLevelChoosingPage();
  void HandleLevelChoosingInput();

  void LoadMap(int index);
};
