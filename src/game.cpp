
#include "game.h"
#include "level.h"
#include <string>
// #include <raygui.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Check if point is inside a rounded rectangle
#define BUTTON_WIDTH  200
#define BUTTON_HEIGHT 100
#define MOVING_BG     0 
#define CUSTOM_STYLE     0 

Game::Game() 
    : prevScreenWidth(GetScreenWidth()), prevScreenHeight(GetScreenHeight()), page(0), 
      startButton(
          Rectangle{ 
            (float)GetScreenWidth() / 2 - BUTTON_WIDTH * 2, 
            (float)GetScreenHeight() / 2, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT 
          },
          "Play Offline"
        ),
      oneVsOneButton(
          Rectangle{ 
            (float)GetScreenWidth() / 2 - BUTTON_WIDTH / 2, 
            (float)GetScreenHeight() / 2, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT 
          },
          "1 vs 1"
        ),
      onlineButton(
          Rectangle{ 
            (float)GetScreenWidth() / 2 + BUTTON_WIDTH, 
            (float)GetScreenHeight() / 2, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT 
          },
          "Play Online"
        ),
      returnButton(
          Rectangle{ 
            (float)GetScreenWidth() / 2 - BUTTON_WIDTH / 2, 
            (float)GetScreenHeight() / 2 + (float)BUTTON_HEIGHT / 1.5f, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT / 2 
          },
          "Return"
        ),
      bgOffsetX(0.0f), speed(100.0f)
{
  background   = getTexture(t_BG);
  textureTitle = getTexture(t_TITLE);

  GuiLoadStyle("./src/style_sunny.rgs");
  GuiSetStyle(DEFAULT, TEXT_SIZE, 22);

}

Game::~Game(){}

// This function is used to initialize the map with the map level 
void Game::LoadMap(int index)
{
  if(index == -1){
    map = std::make_shared<Map1vs1>(GetOneVsOneLevel());
  }else{
    map = std::make_shared<Map>(GetLevel(index));
  }
  map->Initialize(map);
}

void Game::Draw()
{
  switch(page){
    case 0:{
      DrawBg();
      DrawStartingPage();
      break;
    }
    case 1:{
      DrawBg();
      DrawLevelChoosingPage();
      break;
    }
    case 2:{
      ClearBackground(Color{20, 160, 133, 255});
      map->Draw();
      if(map->AllEnemiesDied()){
        map->DrawLevelEndUI("Level Cleared");
        HandleChanges(true);
        returnButton.Draw();
      }
      else if(map->IfPlayerDied()){
        if(map->level.type == LvlType::OFFLINE){
          map->DrawLevelEndUI("Level Failed");
        }else if(map->level.type == LvlType::ONE_VS_ONE){
          if(map->players.at(map->playerId)->alive)
            map->DrawLevelEndUI("Player 1 Wins");
          else 
            map->DrawLevelEndUI("Player 2 Wins");

        }
        HandleChanges(true);
        returnButton.Draw();
      }
      break;
    }

  }
}

void Game::MoveBg() 
{
  bgOffsetX -= speed * GetFrameTime();  // Move left
  if (bgOffsetX <= -background.width) bgOffsetX = 0;
}

void Game::DrawBg()
{
  if(MOVING_BG == 1){
    DrawTexture(background, (int)bgOffsetX, 0, WHITE);
    DrawTexture(background, (int)bgOffsetX + background.width, 0, WHITE);
  }
  else
  {
    Rectangle source = { 0, 0, (float)background.width, (float)background.height }; // Full texture
    Rectangle dest = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }; // Fit to screen

    DrawTexturePro(background, source, dest, { 0, 0 }, 0.0f, WHITE); // Scale to screen size
  }

}


void Game::HandleChanges(bool definitely)
{
  int currScreenWidth  = GetScreenWidth();
  int currScreenHeight = GetScreenHeight();

  if(definitely || (prevScreenWidth != currScreenWidth || prevScreenHeight != currScreenHeight)){
    prevScreenWidth = currScreenWidth;
    prevScreenHeight = currScreenHeight;

    // TODO: Change button positions

    switch (page)
    {
      case 0:{

        startButton.Reinitialise(Rectangle{ 
            (float)currScreenWidth / 2 - BUTTON_WIDTH * 2, 
            (float)currScreenHeight / 2, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT 
          });

        oneVsOneButton.Reinitialise(
            Rectangle{ 
              (float)currScreenWidth / 2 - BUTTON_WIDTH / 2, 
              (float)currScreenHeight / 2, 
              BUTTON_WIDTH, 
              BUTTON_HEIGHT 
            }
          );
        onlineButton.Reinitialise(
            Rectangle{ 
              (float)currScreenWidth / 2 + BUTTON_WIDTH, 
              (float)currScreenHeight / 2, 
              BUTTON_WIDTH, 
              BUTTON_HEIGHT 
            }
          );

        break;
      }
      case 1:{

        size_t numberOfLevel = lvlBtnArr.size();
        int partitions = (numberOfLevel*2)+1;
        float partitionX = currScreenWidth / partitions;

        for(size_t i = 0; i < numberOfLevel; i++){
          lvlBtnArr[i].button.Reinitialise(Rectangle {
            partitionX * (i*2+1),                   // 1, 3, 5
            (float)currScreenHeight/2 - BUTTON_HEIGHT/2,
            partitionX,
            BUTTON_HEIGHT
          });
        }
        break;
      }
      case 2:{
        returnButton.Reinitialise(Rectangle{ 
            (float)currScreenWidth / 2 - BUTTON_WIDTH / 2, 
            (float)currScreenHeight / 2 + (float)BUTTON_HEIGHT / 1.5f, 
            BUTTON_WIDTH, 
            BUTTON_HEIGHT / 2 
          });
        break;
      }
    }

    
  }
  
}

void Game::HandleInput()
{
  switch (page)
  {
  case 0:{
      if(MOVING_BG == 1){
        MoveBg();
      }
      if(CUSTOM_STYLE == 1){
        HandleStartingInput();
      }

      break;
    }
    case 1:{
      if(MOVING_BG == 1){
        MoveBg();
      }
      if(CUSTOM_STYLE == 1){
        HandleLevelChoosingInput();
      }

      break;
    }
    case 2:{
      map->ExplodeBomb();
      map->HandleInput();
      map->MoveCamera();
      if(map->AllEnemiesDied() || map->IfPlayerDied()){
        Vector2 p = GetMousePosition();
  
        if(CheckCollisionPointButton(p, returnButton)){
          returnButton.AnimateStart();
          if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            page = 1;
            LoadLevelChoosingPage();
          }
        }else{
          returnButton.AnimateReturn();
        }
      }
      break;
    }
  
  default:
    break;
  }
  
}

void Game::DrawStartingPage()
{
  // RenderTexture2D tex;
  
  float ratio = (GetScreenHeight() * .5f)/textureTitle.height;

  DrawTextureEx(textureTitle, {(GetScreenWidth() - textureTitle.width * ratio)/2.0f, 0}, 0, ratio, WHITE);

  if(CUSTOM_STYLE == 1){

    startButton.Draw();
    oneVsOneButton.Draw();
    onlineButton.Draw();
  }else{
    if(GuiButton(Rectangle{ 
              (float)GetScreenWidth() / 2 - BUTTON_WIDTH * 2, 
              (float)GetScreenHeight() / 2, 
              BUTTON_WIDTH, 
              BUTTON_HEIGHT 
            },
            "Play Offline")
          ){
            page = 1;
            LoadLevelChoosingPage();
          }

    if(GuiButton(Rectangle{ 
              (float)GetScreenWidth() / 2 - BUTTON_WIDTH / 2, 
              (float)GetScreenHeight() / 2, 
              BUTTON_WIDTH, 
              BUTTON_HEIGHT 
            },
            "1 vs 1")
          ){
            page = 2;
            LoadMap(-1);
          }

    if(GuiButton(Rectangle{ 
              (float)GetScreenWidth() / 2 + BUTTON_WIDTH, 
              (float)GetScreenHeight() / 2, 
              BUTTON_WIDTH, 
              BUTTON_HEIGHT 
            },
            "Play Online")
          ){
            // page = 2;
            // LoadMap(-1);
          }
  }

  

}

void Game::HandleStartingInput()
{ 
  Vector2 p = GetMousePosition();
  
  if(CheckCollisionPointButton(p, startButton)){
    startButton.AnimateStart();
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
      page = 1;
      LoadLevelChoosingPage();
    }
  }else{
    startButton.AnimateReturn();
    // std::cout << "OUT\n";
  }

  if(CheckCollisionPointButton(p, oneVsOneButton)){
    oneVsOneButton.AnimateStart();
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
      page = 2;
      LoadMap(-1);
      // LoadLevelChoosingPage();
    }
  }else{
    oneVsOneButton.AnimateReturn();
  }

  if(CheckCollisionPointButton(p, onlineButton)){
    onlineButton.AnimateStart();
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
      // page = 1;
      // LoadLevelChoosingPage();
    }
  }else{
    onlineButton.AnimateReturn();
  }
}

void Game::LoadLevelChoosingPage()
{
  int numberOfLevel = 6;
  if(lvlBtnArr.size() == numberOfLevel) return;
  int partitions = (numberOfLevel*2)+1;

  int screenWidth  = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  float partitionX = screenWidth / partitions;
  // float partitionY = screenHeight / partitions;

  for(int i = 0; i < numberOfLevel; i++){
    
    std::string text;
    if(i == 0){
      text = "Random";
    }else{
      text = "Level " + std::to_string(i);
    }

    lvlBtnArr.push_back(LevelButton{Rectangle {
      partitionX * (i*2+1),                   // 1, 3, 5
      (float)screenHeight/2 - BUTTON_HEIGHT/2,
      partitionX,
      BUTTON_HEIGHT
    },text,i,i});

  }
}

void Game::DrawLevelChoosingPage()
{ 
  if(CUSTOM_STYLE == 1){
    for(auto& lbtn: lvlBtnArr){
      lbtn.button.Draw();
    }
  }else{
    int numberOfLevel = 6;
    int partitions = (numberOfLevel*2)+1;

    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float partitionX = screenWidth / partitions;

    for(int i = 0; i < numberOfLevel; i++){
      
      std::string text;
      if(i == 0){
        text = "Random";
      }else{
        text = "Level " + std::to_string(i);
      }


      if(GuiButton(Rectangle {
                  partitionX * (i*2+1),                   // 1, 3, 5
                  (float)screenHeight/2 - BUTTON_HEIGHT/2,
                  partitionX,
                  BUTTON_HEIGHT
                },text.c_str())
            ){
              loadLevels();
              page = 2;
              LoadMap(i);
            }
    }
  }
}

void Game::HandleLevelChoosingInput()
{
  Vector2 p = GetMousePosition();
  for(auto& lbtn: lvlBtnArr){
    if(CheckCollisionPointButton(p, lbtn.button)){
      lbtn.button.AnimateStart();
      if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        loadLevels();
        page = 2;
        LoadMap(lbtn.level);
      }
    }else{
      lbtn.button.AnimateReturn();
    }
  }
}


