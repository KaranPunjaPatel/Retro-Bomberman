
#include <string>

// #include <raygui.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "game.h"
#include "level.h"

constexpr int BUTTON_WIDTH = 200;
constexpr int BUTTON_HEIGHT = 100;

Game::Game()
    : prevScreenWidth(GetScreenWidth()), prevScreenHeight(GetScreenHeight()), page(0),
      returnButton(
          Rectangle{
              (float)GetScreenWidth() / 2 - BUTTON_WIDTH / 2,
              (float)GetScreenHeight() / 2 + (float)BUTTON_HEIGHT / 1.5f,
              BUTTON_WIDTH,
              BUTTON_HEIGHT / 2},
          "Return")
{

  background = getTexture(t_BG);
  textureTitle = getTexture(t_TITLE);

  SetTextureFilter(background, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(textureTitle, TEXTURE_FILTER_BILINEAR);

  GuiLoadStyle("./src/style_sunny.rgs");
  GuiSetStyle(DEFAULT, TEXT_SIZE, 22);
}

Game::~Game() {}

void Game::LoadMap(int index)
{
  std::cout << "[DEBUG] Loading map for index: " << index << std::endl;
  if (index == -1)
  {
    map = std::make_shared<Map1vs1>(GetOneVsOneLevel());
  }
  else
  {
    map = std::make_shared<Map>(GetLevel(index));
  }
  std::cout << "[DEBUG] Map created, initializing..." << std::endl;
  map->Initialize(map);
  std::cout << "[DEBUG] Map initialized successfully!" << std::endl;
}

void Game::Draw()
{
  switch (page)
  {
  case 0:
  {
    DrawBg();
    DrawStartingPage();
    break;
  }
  case 1:
  {
    DrawBg();
    DrawLevelChoosingPage();
    break;
  }
  case 2:
  {
    ClearBackground(Color{20, 160, 133, 255});
    // std::cout << "[GAME] 4. Map Draw Start" << std::endl;
    map->Draw();
    // std::cout << "[GAME] 5. Map Draw End" << std::endl;

    if (GuiButton(Rectangle{(float)GetScreenWidth() - 120, 50, 100, 40}, "Exit") || IsKeyPressed(KEY_ESCAPE))
    {
      page = 0; // Return to Main Menu
      return;
    }

    if (map->AllEnemiesDied())
    {
      map->DrawLevelEndUI("Level Cleared");
      HandleChanges(true);
      returnButton.Draw();
    }
    else if (map->IfPlayerDied())
    {
      if (map->level.type == LvlType::OFFLINE)
      {
        map->DrawLevelEndUI("Level Failed");
      }
      else if (map->level.type == LvlType::ONE_VS_ONE)
      {
        if (map->players.at(map->playerId)->IsAlive())
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

void Game::DrawBg()
{
  // Rectangle source = {0, 0, (float)background.width, (float)background.height};
  // Rectangle dest = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
  // DrawTexturePro(background, source, dest, {0, 0}, 0.0f, WHITE);

  float screenW = (float)GetScreenWidth();
  float screenH = (float)GetScreenHeight();
  float bgW = (float)background.width;
  float bgH = (float)background.height;
  // Calculate scale factor to cover the entire screen without stretching ("Aspect Cover")
  float scale = std::max(screenW / bgW, screenH / bgH);
  float destW = bgW * scale;
  float destH = bgH * scale;
  // Center the background image
  float destX = (screenW - destW) / 2.0f;
  float destY = (screenH - destH) / 2.0f;
  Rectangle source = {0.0f, 0.0f, bgW, bgH};
  Rectangle dest = {destX, destY, destW, destH};
  DrawTexturePro(background, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

void Game::HandleChanges(bool definitely)
{
  int currScreenWidth = GetScreenWidth();
  int currScreenHeight = GetScreenHeight();

  if (definitely || (prevScreenWidth != currScreenWidth || prevScreenHeight != currScreenHeight))
  {
    prevScreenWidth = currScreenWidth;
    prevScreenHeight = currScreenHeight;

    // TODO: Change button positions

    switch (page)
    {
    case 2:
    {
      returnButton.Reinitialise(Rectangle{
          (float)currScreenWidth / 2 - BUTTON_WIDTH / 2,
          (float)currScreenHeight / 2 + (float)BUTTON_HEIGHT / 1.5f,
          BUTTON_WIDTH,
          BUTTON_HEIGHT / 2});
      break;
    }
    }
  }
}

void Game::HandleInput()
{
  switch (page)
  {
  case 2:
  {
    // std::cout << "[GAME] 1. ExplodeBomb" << std::endl;
    map->ExplodeBomb();
    // std::cout << "[GAME] 2. Map HandleInput" << std::endl;
    map->HandleInput();
    // std::cout << "[GAME] 3. MoveCamera" << std::endl;
    map->MoveCamera();
    if (map->AllEnemiesDied() || map->IfPlayerDied())
    {
      Vector2 p = GetMousePosition();

      if (CheckCollisionPointButton(p, returnButton))
      {
        returnButton.AnimateStart();
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
          page = 1;
        }
      }
      else
      {
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

  // float ratio = (GetScreenHeight() * .5f) / textureTitle.height;

  // DrawTextureEx(textureTitle, {(GetScreenWidth() - textureTitle.width * ratio) / 2.0f, 0}, 0, ratio, WHITE);

  if (GuiButton(Rectangle{
                    (float)GetScreenWidth() / 2 - BUTTON_WIDTH * 2,
                    (float)GetScreenHeight() / 2,
                    BUTTON_WIDTH,
                    BUTTON_HEIGHT},
                "Play Offline"))
  {
    page = 1;
  }

  if (GuiButton(Rectangle{
                    (float)GetScreenWidth() / 2 - BUTTON_WIDTH / 2,
                    (float)GetScreenHeight() / 2,
                    BUTTON_WIDTH,
                    BUTTON_HEIGHT},
                "1 vs 1"))
  {
    page = 2;
    LoadMap(-1);
  }

  if (GuiButton(Rectangle{
                    (float)GetScreenWidth() / 2 + BUTTON_WIDTH,
                    (float)GetScreenHeight() / 2,
                    BUTTON_WIDTH,
                    BUTTON_HEIGHT},
                "Play Online"))
  {
  }
}

void Game::DrawLevelChoosingPage()
{
  if (GuiButton(Rectangle{20, 20, 100, 40}, "< Back") || IsKeyPressed(KEY_ESCAPE))
  {
    page = 0;
    return;
  }

  const size_t numberOfLevel = 6;
  int partitions = (numberOfLevel * 2) + 1;

  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  float partitionX = screenWidth / partitions;

  for (int i = 0; i < (int)numberOfLevel; i++)
  {
    std::string text = (i == 0) ? "Random" : "Level " + std::to_string(i);

    if (GuiButton(Rectangle{
                      partitionX * (i * 2 + 1),
                      (float)screenHeight / 2 - BUTTON_HEIGHT / 2,
                      partitionX,
                      BUTTON_HEIGHT},
                  text.c_str()))
    {
      loadLevels();
      page = 2;
      LoadMap(i);
      break;
    }
  }
}
