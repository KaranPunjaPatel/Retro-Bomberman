#pragma once

#include <vector> 
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>

#include <raylib.h> 

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

  Button returnButton;

  Texture2D background;


public:
  Game();
  ~Game();
  
  void DrawBg();

  void HandleChanges(bool definitely);

  void Draw();
  void HandleInput();

  void DrawStartingPage();

  void DrawLevelChoosingPage();

  void LoadMap(int index);
};
