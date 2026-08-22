
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "macros.h"
#include "map.h"
#include "util.h"

Map::Map(Level level) : level(level), stopFlag(false), explodeBomb(false), camera({0})
{
  std::cout << "[DEBUG] Map Constructor Start" << std::endl;
  numRows = level.rows;
  numCols = level.columns;
  cellSize = BLOCK_SIZE;

  mapWidth = cellSize * numCols;
  mapHeight = cellSize * numRows;

  startTime = GetTime();
  finishTime = 0.0;

  camera.target = (Vector2){0, 0}; // Point the camera follows
  // camera.offset = (Vector2){ screenWidth / 2, screenHeight / 2 }; // Camera center
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  // camera3d.

  camera3d = {0};
  camera3d.position = {0.0f, 20.0f, 10.f};
  camera3d.target = {0.0f, 0.0f, 0.0f};
  camera3d.up = {0.0f, 1.0f, 0.0f};
  camera3d.fovy = 45.0f;
  camera3d.projection = CAMERA_PERSPECTIVE;

  grass[0] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));
  grass[1] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));
  grass[2] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));

  grass[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS1);
  grass[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS2);
  grass[2].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS3);

  boundaryModel = getModel(m_BOUNDARY);

  std::cout << "[DEBUG] Map Constructor End" << std::endl;
}

Map::~Map()
{
  stopFlag.store(true, std::memory_order_release);
  if (bombCountdown.joinable())
  {
    bombCountdown.join();
  }
  UnloadModel(grass[0]);
  UnloadModel(grass[1]);
  UnloadModel(grass[2]);
}

void Map::StartBombCountdownThread()
{
  bombCountdown = std::thread([this] { // Thread runs to countdown the bomb timer's
    while (!stopFlag.load(std::memory_order_acquire))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      std::lock_guard<std::mutex> lock(bombsMutex);
      if (bombsArr.empty())
        continue;

      for (auto &bomb : bombsArr)
      {
        if (bomb != nullptr && bomb->CountDown())
        {
          explodeBomb.store(true); // Set the explosion flag if bomb countdown reaches zero
        }
      }
    }
  });
}

void Map::Initialize(std::shared_ptr<Map> mapPtr)
{
  std::cout << "[DEBUG] Map::Initialize Start" << std::endl;
  // Create the map according to size
  map.resize(numRows);
  for (size_t i = 0; i < numRows; i++)
  {
    map[i].resize(numCols);
  }

  // Assign each block of map its type
  if (SHOW_2D == 1)
  {
    for (size_t i = 0; i < numRows; i++)
    {
      for (size_t j = 0; j < numCols; j++)
      {

        map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER);
      }
    }
  }
  else
  {
    // loadAnimations();
    // camera3d.position = BLOCK_SIZE_3D
    BoundingBox box = GetModelBoundingBox(*boundaryModel);

    float modelHeight = box.max.y - box.min.y;

    if (modelHeight < BLOCK_SIZE_3D)
    {
      Matrix scaleMatrix = MatrixScale(1.0f, BLOCK_SIZE_3D / modelHeight, 1.0f);
      (*boundaryModel).transform = MatrixMultiply((*boundaryModel).transform, scaleMatrix);
    }

    for (size_t i = 0; i < numRows; i++)
    {
      for (size_t j = 0; j < numCols; j++)
      {
        int random = GetRandomValue(0, 50);
        if (random < 40)
        {
          map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[0]);
        }
        else if (random < 45)
        {
          map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[1]);
        }
        else
        {
          map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[2]);
        }
      }
    }
  }

  // Assign powerups to blocks
  for (size_t i = 0; i < level.powerUps.size(); i++)
  {
    int row = level.powerUps[i].row;
    int column = level.powerUps[i].column;

    map[row][column].SetPowerType(level.powerUps[i].type);
  }

  // players.resize(1);
  playerId = GetRandomValue(0, INT_MAX);
  std::cout << "[DEBUG] Creating Player..." << std::endl;
  players.insert({playerId, std::make_shared<Player>(mapPtr, playerId, 0, 0)});

  players.at(playerId)->SetSpeed(level.speed);
  players.at(playerId)->SetBombCount(level.bombCount);
  players.at(playerId)->SetBombRadius(level.bombRadius);

  for (auto pos : level.enemies)
  {
    int id = GetRandomValue(0, INT_MAX);
    std::cout << "[DEBUG] Creating Enemy ID " << id << "..." << std::endl;
    enemies.insert({id, std::make_shared<Enemy>(mapPtr, id, pos.row, pos.column)});
    enemies.at(id)->SetSpeed(level.speed);
    enemies.at(id)->SetDirection(pos.direction);
  }

  std::cout << "[DEBUG] Starting Bomb Countdown Thread..." << std::endl;
  StartBombCountdownThread();
  std::cout << "[DEBUG] Map::Initialize End" << std::endl;
}

void Map::Print()
{
  for (size_t row = 0; row < numRows; row++)
  {
    for (size_t column = 0; column < numCols; column++)
    {
      std::cout << map[row][column].type << " ";
    }
    std::cout << std::endl;
  }
}

void Map::Draw()
{
  // DrawRectangle(-30,-30,mapWidth+60,mapHeight+60,WHITE);

  if (SHOW_2D == 1)
  {

    BeginMode2D(camera);

    for (size_t row = 0; row < numRows; row++)
    {
      for (size_t column = 0; column < numCols; column++)
      {
        map[row][column].Draw();
      }
    }

    {
      std::lock_guard<std::mutex> lock(bombsMutex);
      for (auto &bomb : bombsArr)
      {
        if (bomb != nullptr)
        {
          bomb->Draw();
        }
      }
    }

    for (auto it = players.begin(); it != players.end(); ++it)
    {
      if (it->second->IsAlive())
      {
        it->second->Draw();
      }
    }
    for (auto it = enemies.begin(); it != enemies.end(); ++it)
    {
      if (it->second->IsAlive())
      {
        it->second->Draw();
      }
    }

    EndMode2D();
  }
  else
  {
    // std::cout << "  [MAP] BeginMode3D..." << std::endl;
    BeginMode3D(camera3d);

    DrawMapBoundary(numCols, numRows);
    // std::cout << "  [MAP] Drawing Blocks..." << std::endl;
    for (size_t row = 0; row < numRows; row++)
    {
      for (size_t column = 0; column < numCols; column++)
      {
        map[row][column].Draw3d();
      }
    }

    {
      // std::cout << "  [MAP] Drawing Bombs..." << std::endl;
      std::lock_guard<std::mutex> lock(bombsMutex);
      for (auto &bomb : bombsArr)
      {
        if (bomb != nullptr)
        {
          bomb->Draw3d();
        }
      }
    }

    // std::cout << "  [MAP] Drawing Player 3D..." << std::endl;
    for (auto it = players.begin(); it != players.end(); ++it)
    {
      if (it->second->IsAlive())
      {
        it->second->Draw3d();
      }
    }

    // std::cout << "  [MAP] Drawing Enemies 3D..." << std::endl;
    for (auto it = enemies.begin(); it != enemies.end(); ++it)
    {
      if (it->second->IsAlive())
      {
        it->second->Draw3d();
      }
    }
    // std::cout << "  [MAP] EndMode3D..." << std::endl;
    EndMode3D();
  }
  DrawUI();
}

void Map::DrawUI()
{
  std::shared_ptr<Player> player = players.at(playerId);
  {
    DrawTexturePro(getTexture(t_COUNT),
                   Rectangle{
                       0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                   Rectangle{
                       UI_OFFSET,
                       UI_OFFSET,
                       UI_TEXTURE_SIZE,
                       UI_TEXTURE_SIZE},
                   Vector2{0, 0},
                   0.0f, WHITE);

    DrawText(
        std::to_string(player->GetBombCount()).c_str(),
        UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET,
        UI_OFFSET,
        UI_FONT_SIZE,
        BLACK);

    int tWidth = MeasureText(std::to_string(player->GetBombCount()).c_str(), UI_FONT_SIZE);

    DrawTexturePro(getTexture(t_RADIUS),
                   Rectangle{
                       0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                   Rectangle{
                       UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth + UI_OFFSET,
                       UI_OFFSET,
                       UI_TEXTURE_SIZE,
                       UI_TEXTURE_SIZE},
                   Vector2{0, 0},
                   0.0f, WHITE);

    DrawText(
        std::to_string(player->GetBombRadius()).c_str(),
        UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET,
        UI_OFFSET,
        UI_FONT_SIZE,
        BLACK);
  }
  {
    int screenWidth = GetScreenWidth();
    int tWidth = MeasureText(std::to_string(player->lifeCount).c_str(), UI_FONT_SIZE);

    DrawText(
        std::to_string(player->lifeCount).c_str(),
        screenWidth - (UI_OFFSET + tWidth),
        UI_OFFSET,
        UI_FONT_SIZE,
        BLACK);

    DrawTexturePro(getTexture(t_HEART),
                   Rectangle{
                       0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                   Rectangle{
                       screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE),
                       UI_OFFSET,
                       UI_TEXTURE_SIZE,
                       UI_TEXTURE_SIZE},
                   Vector2{0, 0},
                   0.0f, WHITE);

    char time[6];
    if (finishTime != 0.0)
    {
      getTime(finishTime - startTime, time);
    }
    else
    {
      getTime(GetTime() - startTime, time);
    }

    int tWidth2 = MeasureText(time, UI_FONT_SIZE);

    DrawText(
        time,
        screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth2),
        UI_OFFSET,
        UI_FONT_SIZE,
        BLACK);

    DrawTexturePro(getTexture(t_CLOCK),
                   Rectangle{
                       0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                   Rectangle{
                       screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth2 + UI_OFFSET + UI_TEXTURE_SIZE),
                       UI_OFFSET,
                       UI_TEXTURE_SIZE,
                       UI_TEXTURE_SIZE},
                   Vector2{0, 0},
                   0.0f, WHITE);
  }
}

void Map::DrawLevelEndUI(const char *text)
{
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  const float cardWidth = 460.0f;
  const float cardHeight = 280.0f;

  float rectX = ((float)screenWidth - cardWidth) / 2.0f;
  float rectY = ((float)screenHeight - cardHeight) / 2.0f;

  Rectangle rect = {rectX, rectY, cardWidth, cardHeight};

  DrawRectangleRoundedLinesEx(rect, 0.5f, 2, 5.0f, SKYBLUE);
  DrawRectangleRounded(rect, 0.5f, 2, BLUE);

  int titleFontSize = 32;
  int tWidth = MeasureText(text, titleFontSize);
  DrawText(
      text,
      (int)(rectX + (cardWidth - tWidth) / 2.0f),
      (int)(rectY + 25),
      titleFontSize,
      YELLOW);

  float row1Y = rectY + 85.0f;
  DrawTexturePro(getTexture(t_CLOCK),
                 Rectangle{0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                 Rectangle{rectX + 40.0f, row1Y, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                 Vector2{0, 0},
                 0.0f, WHITE);

  DrawText("Time :", (int)(rectX + 90.0f), (int)row1Y, UI_FONT_SIZE, WHITE);

  if (finishTime == 0.0f)
  {
    finishTime = GetTime();
  }

  char timeStr[16];
  getTime(finishTime - startTime, timeStr);

  DrawText(timeStr, (int)(rectX + 290.0f), (int)row1Y, UI_FONT_SIZE, WHITE);

  float row2Y = rectY + 145.0f;

  DrawTexturePro(getTexture(t_HEART),
                 Rectangle{0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                 Rectangle{rectX + 40.0f, row2Y, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE},
                 Vector2{0, 0},
                 0.0f, WHITE);

  DrawText("Lives left :", (int)(rectX + 90.0f), (int)row2Y, UI_FONT_SIZE, WHITE);

  int lives = (players.find(playerId) != players.end()) ? players.at(playerId)->lifeCount : 0;

  DrawText(std::to_string(lives).c_str(), (int)(rectX + 290.0f), (int)row2Y, UI_FONT_SIZE, WHITE);
}

void Map::DrawMapBoundary(float width, float length)
{
  BoundingBox box = GetModelBoundingBox(*boundaryModel);

  float modelWidth = box.max.x - box.min.x;
  float modelLength = box.max.z - box.min.z;

  float tempwidth = 0;

  DrawModel(*boundaryModel, Vector3{-modelWidth / 2, 0, -modelLength / 2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{-modelWidth / 2, 0, length + modelLength / 2}, 1, WHITE);

  while (tempwidth <= width)
  {
    tempwidth += modelWidth / 1.5;
    DrawModel(*boundaryModel, Vector3{tempwidth - modelWidth / 2, 0, -modelLength / 2}, 1, WHITE);

    DrawModel(*boundaryModel, Vector3{tempwidth - modelWidth / 2, 0, length + modelLength / 2}, 1, WHITE);
  }
  DrawModel(*boundaryModel, Vector3{width + modelWidth / 2, 0, -modelLength / 2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{width + modelWidth / 2, 0, length + modelLength / 2}, 1, WHITE);

  float templength = 0;

  while (templength <= length)
  {
    templength += modelLength / 1.5;
    DrawModel(*boundaryModel, Vector3{-modelWidth / 2, 0, templength - modelLength / 2}, 1, WHITE);
    DrawModel(*boundaryModel, Vector3{width + modelWidth / 2, 0, templength - modelLength / 2}, 1, WHITE);
  }
  DrawModel(*boundaryModel, Vector3{-modelWidth / 2, 0, length + modelLength / 2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{width + modelWidth / 2, 0, length + modelLength / 2}, 1, WHITE);
}

void Map::MoveCamera()
{
  Rectangle rect = players.at(playerId)->GetRectangle();
  if (SHOW_2D == 1)
  {

    int centerX = rect.x + rect.width / 2;
    int centerY = rect.y + rect.height / 2;

    int width = GetScreenWidth();
    int height = GetScreenHeight();

    camera.offset = Vector2{(float)(width / 2), (float)(height / 2)};

    float halfViewWidth = (width / 2.0f) / camera.zoom;   // Scale based on zoom
    float halfViewHeight = (height / 2.0f) / camera.zoom; // Scale based on zoom

    if (centerX + halfViewWidth > mapWidth)
    {
      camera.target.x = mapWidth - halfViewWidth + MAP_BORDER;
    }
    else if (centerX - halfViewWidth < 0)
    {
      camera.target.x = halfViewWidth - MAP_BORDER;
    }
    else
    {
      camera.target.x = centerX;
    }

    if (centerY + halfViewHeight > mapHeight)
    {
      camera.target.y = mapHeight - halfViewHeight + MAP_BORDER;
    }
    else if (centerY - halfViewHeight < 0)
    {
      camera.target.y = halfViewHeight - MAP_BORDER;
    }
    else
    {
      camera.target.y = centerY;
    }

    // Zoom in and out with mouse wheel
    float zoomFactor = GetMouseWheelMove();
    if (zoomFactor != 0)
    {
      camera.zoom += zoomFactor * 0.05f;
      camera3d.fovy += zoomFactor * 2.0f;
      // std::cout << "Change\n";
    }
    // Prevent zoom from going too small or too big
    if (camera.zoom < 0.5f)
      camera.zoom = 0.5f;

    if (camera.zoom > 3.0f)
      camera.zoom = 3.0f;

    float visibleWidth = width / camera.zoom;
    float visibleHeight = height / camera.zoom;
    if (visibleWidth > mapWidth + MAP_BORDER * 2)
    { // If can see entire map then camera target is middle of map
      camera.target.x = (float)(mapWidth) / 2;
    }
    if (visibleHeight > mapHeight + MAP_BORDER * 2)
    {
      camera.target.y = (float)(mapHeight) / 2;
    }
  }
  else
  {
    float cx = (rect.x + rect.width / 2) / BLOCK_SIZE;
    float cz = (rect.y + rect.height / 2) / BLOCK_SIZE;
    camera3d.target = {cx, 0.5f, cz};
    camera3d.position = {cx, 17.0f, cz + 7.0f};
  }
}

bool Map::IsColliding(int id)
{
  auto playerIt = players.find(id);
  if (playerIt == players.end())
    return true;

  auto player = playerIt->second;

  Rectangle rect = player->GetRectangle();
  int row = player->GetRowPos();
  int column = player->GetColumnPos();

  int adjacentRows[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
  int adjacentColumns[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

  for (size_t i = 0; i < 9; ++i)
  {
    int tempRow = row + adjacentRows[i];
    int tempColumn = column + adjacentColumns[i];

    if (WithinBoundary(tempRow, tempColumn) && map[tempRow][tempColumn].IsBlock())
    {
      Rectangle blockHitbox{
          .x = (float)BLOCK_SIZE * tempColumn + BLOCK_HITBOX_PADDING,
          .y = (float)BLOCK_SIZE * tempRow + BLOCK_HITBOX_PADDING,
          .width = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2,
          .height = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2};

      if (CheckCollisionRecs(rect, blockHitbox))
        return true;
    }
  }

  {
    std::lock_guard<std::mutex> lock(bombsMutex);

    // Loop through bombs ONCE, not 9 times
    for (auto &bomb : bombsArr)
    {
      if (bomb == nullptr || bomb->IsIdPresent(id))
        continue;

      // Optional optimization: If the bomb is too far away, skip it!
      if (abs(bomb->GetRow() - row) > 1 || abs(bomb->GetColumn() - column) > 1)
        continue;

      Rectangle bombHitbox{
          .x = (float)BLOCK_SIZE * bomb->GetColumn(),
          .y = (float)BLOCK_SIZE * bomb->GetRow(),
          .width = BLOCK_SIZE,
          .height = BLOCK_SIZE};

      if (CheckCollisionRecs(rect, bombHitbox))
        return true;
    }
  }

  return false;
}

bool Map::WithinBoundary(int row, int column)
{
  return (row >= 0 && row < (int)numRows && column >= 0 && column < (int)numCols);
}

void Map::ChangeBombIds(int id)
{
  std::shared_ptr<Human> human;

  auto playerIt = players.find(id);
  if (playerIt != players.end())
  {
    human = playerIt->second;
  }
  else
  {
    auto enemyIt = enemies.find(id);
    if (enemyIt != enemies.end())
    {
      human = enemyIt->second;
    }
    else
    {
      return; // ID not found in players or enemies
    }
  }

  static constexpr int adjacentRows[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
  static constexpr int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

  std::lock_guard<std::mutex> lock(bombsMutex);
  for (int i = 0; i < 8; ++i)
  {
    int newRow = human->GetRowPos() + adjacentRows[i];
    int newCol = human->GetColumnPos() + adjacentColumns[i];

    if (!WithinBoundary(newRow, newCol))
    {
      continue;
    }

    Rectangle blockRec{
        .x = (float)BLOCK_SIZE * newCol,
        .y = (float)BLOCK_SIZE * newRow,
        .width = (float)BLOCK_SIZE,
        .height = (float)BLOCK_SIZE};

    for (auto &bomb : bombsArr)
    {
      // Early continues for cleaner logical checks
      if (!bomb)
        continue;
      if (!bomb->HasSamePosition(newRow, newCol))
        continue;
      if (!bomb->IsIdPresent(id))
        continue;

      if (!CheckCollisionRecs(human->GetRectangle(), blockRec))
      {
        bomb->BombIdsRemove(id);
      }
    }
  }
}

void Map::HandleInput()
{
  // for (auto it = players.begin(); it != players.end(); ++it) {
  //   it->second->HandleInput();
  // }
  try
  {

    // std::cout << "  [MAP] Player HandleInput..." << std::endl;
    if (players.at(playerId)->IsAlive())
      players.at(playerId)->HandleInput();

    // std::cout << "  [MAP] Enemies Update..." << std::endl;
    for (auto it = enemies.begin(); it != enemies.end(); ++it)
    {
      if (it->second->IsAlive())
      {
        it->second->Move();
      }
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "  [EXCEPTION IN MAP::HANDLEINPUT] " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "  [UNKNOWN CRASH IN MAP::HANDLEINPUT]" << std::endl;
  }
}

bool Map::AlreadyHasBomb(size_t row, size_t column)
{
  {
    std::lock_guard<std::mutex> lock(bombsMutex);
    for (auto &bomb : bombsArr)
    {
      if (bomb != nullptr && bomb->HasSamePosition(row, column))
      {
        return true;
      }
    }
  }
  return false;
}

void Map::AddBomb(int id)
{
  if (players.find(id) == players.end())
    return;

  std::shared_ptr<Player> player = players.at(id);

  std::vector<int> ids;

  /*
  Vector2 circleCenter = {(float)BLOCK_SIZE * column + BLOCK_SIZE/2,
                                  (float)BLOCK_SIZE * row + BLOCK_SIZE/2};
  float radius = (float)BLOCK_SIZE/2.5;
*/

  Rectangle rect = {
      (float)(BLOCK_SIZE * player->GetColumnPos()), (float)(BLOCK_SIZE * player->GetRowPos()), (float)BLOCK_SIZE, (float)BLOCK_SIZE};

  for (auto it = players.begin(); it != players.end(); ++it)
  {
    if (CheckCollisionRecs(rect, it->second->GetRectangle()))
    {
      ids.push_back(it->first);
    }
  }

  for (auto it = enemies.begin(); it != enemies.end(); ++it)
  {
    if (CheckCollisionRecs(rect, it->second->GetRectangle()))
    {
      ids.push_back(it->first);
    }
  }

  bombsArr.push_back(std::make_shared<Bomb>(id, player->GetRowPos(), player->GetColumnPos(), player->GetBombRadius(), ids));
}

void Map::ExplodeBomb()
{
  if (explodeBomb.load())
  {
    std::lock_guard<std::mutex> lock(bombsMutex);

    while (!bombsArr.empty())
    {

      std::shared_ptr<Bomb> bomb = bombsArr.front();
      if (bomb != nullptr && bomb->HasExploded())
      {
        int id = bomb->GetOwner();
        std::shared_ptr<Player> player = players.at(id);

        ExplodeArea(bomb);
        RedrawExplodeArea();
        processedBombsArr.push_back(bomb);
        bombsArr.erase(bombsArr.begin());
        player->ModifyStat(StatType::BombCount, 1);
      }
      else
      {
        break;
      }
    }

    explodeBomb.store(false);
  }

  if (!explodeArr.empty())
  {

    bool res = false;
    // Clears explode area whose time is up
    explodeArr.erase(
        std::remove_if(
            explodeArr.begin(),
            explodeArr.end(),
            [&](const std::pair<int, int> &p)
            {
              if (map[p.first][p.second].ClearExplode())
              {
                map[p.first][p.second].SetType(GRASS_BLOCK);
                res = true;
                return true;
              }

              return false;
            }),
        explodeArr.end());

    if (res)
    {
      RedrawExplodeArea();
    }
  }
}

void Map::ExplodeArea(std::shared_ptr<Bomb> bomb)
{
  int row = bomb->GetRow();
  int column = bomb->GetColumn();
  int radius = bomb->GetBombRadius();

  map[row][column].SetType(EXPLODE);
  map[row][column].SetExplodeTimer();
  explodeArr.push_back(std::make_pair(row, column));
  map[row][column].texturePart = MIDDLE;
  map[row][column].rotation = 0;

  static const int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

  for (const auto &d : dirs)
  {
    for (int i = 1; i <= radius; i++)
    {
      int tempRow = row + d[0] * i;
      int tempCol = column + d[1] * i;

      for (auto &b : bombsArr)
      {
        if (b != nullptr && b->HasSamePosition(tempRow, tempCol))
        {
          b->SetToExplode();
        }
      }

      int res = ExplodeAreaHelper(tempRow, tempCol);
      if (res == 0 || res == 1)
        break;
    }
  }
}

int Map::CheckIfExplosion(int row, int column) // 0 - Out of boundary / Not exploding, 1 - Exploding
{
  if (WithinBoundary(row, column))
  {
    if (map[row][column].type == EXPLODE)
    {
      return 1;
    }
  }
  return 0;
}

int Map::ExplodeAreaHelper(int row, int column) // 0 - Out of boundary, 1 - Clay block broke, 2 - No obsturction, 3 - Was already exploding
{
  if (WithinBoundary(row, column))
  {
    if (map[row][column].IsClay())
    {
      map[row][column].SetType(EXPLODE);
      map[row][column].SetExplodeTimer();
      explodeArr.push_back(std::make_pair(row, column));
      return 1;
    }
    if (map[row][column].type == EXPLODE)
    {
      map[row][column].SetExplodeTimer();
      return 3;
    }
    if (!map[row][column].IsBrick())
    {
      map[row][column].SetType(EXPLODE);
      map[row][column].SetExplodeTimer();
      explodeArr.push_back(std::make_pair(row, column));
      return 2;
    }
  }
  return 0;
}

bool Map::IsInExplosion(int id) // TODO: Work on the die function
{
  // std::shared_ptr<Player> player = players.at(id);

  std::shared_ptr<Human> human;

  if (players.find(id) != players.end())
  {
    human = players.at(id);
  }
  else if (enemies.find(id) != enemies.end())
  {
    human = enemies.at(id);
  }
  else
  {
    return false;
  }

  if (!human->IsAlive())
    return false;

  if (map[human->GetRowPos()][human->GetColumnPos()].type == EXPLODE)
  {
    return true;
  }

  int adjacentRows[8] = {-1, -1, -1, 0, 0, 1, 1, 1};    // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1}; // column offsets

  for (int i = 0; i < 8; ++i)
  {
    int newRow = human->GetRowPos() + adjacentRows[i];
    int newCol = human->GetColumnPos() + adjacentColumns[i];

    if (WithinBoundary(newRow, newCol))
    {
      if (map[newRow][newCol].type == EXPLODE &&
          CheckCollisionRecs(human->GetRectangle(),
                             Rectangle{(float)(BLOCK_SIZE * newCol),
                                       (float)(BLOCK_SIZE * newRow),
                                       (float)BLOCK_SIZE,
                                       (float)BLOCK_SIZE}))
      {
        return true;
      }
    }
  }
  return false;
}

bool Map::IsEnemyColliding(int id)
{
  if (enemies.find(id) == enemies.end())
    return false;
  // std::cout << id << std::endl;
  std::shared_ptr<Enemy> enemy = enemies.at(id);

  if (enemy->GetRectangle().x + enemy->GetRectangle().width > mapWidth ||
      enemy->GetRectangle().x < 0 ||
      enemy->GetRectangle().y + enemy->GetRectangle().height > mapHeight ||
      enemy->GetRectangle().y < 0)
  {
    enemy->IsTouchingBoundary();
    return true;
  }

  switch (enemy->GetDirection())
  {
  case Direction::NORTH:
  {
    int row = enemy->GetRowPos() - 1;
    int column = enemy->GetColumnPos();

    if (WithinBoundary(row, column) &&
        map[row][column].IsBlock() &&
        CheckCollisionRecs(enemy->GetRectangle(), Rectangle{
                                                      .x = (float)BLOCK_SIZE * column + BLOCK_HITBOX_PADDING,
                                                      .y = (float)BLOCK_SIZE * row + BLOCK_HITBOX_PADDING,
                                                      .width = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2,
                                                      .height = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2}))
    {
      enemy->GetRectangle().x = (float)BLOCK_SIZE * (enemy->GetColumnPos());
      enemy->GetRectangle().y = (float)BLOCK_SIZE * (enemy->GetRowPos());
      return true;
    }
    break;
  }
  case Direction::EAST:
  {
    int row = enemy->GetRowPos();
    int column = enemy->GetColumnPos() + 1;
    if (WithinBoundary(row, column) &&
        map[row][column].IsBlock() &&
        CheckCollisionRecs(enemy->GetRectangle(), Rectangle{
                                                      .x = (float)BLOCK_SIZE * column + BLOCK_HITBOX_PADDING,
                                                      .y = (float)BLOCK_SIZE * row + BLOCK_HITBOX_PADDING,
                                                      .width = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2,
                                                      .height = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2}))
    {
      enemy->GetRectangle().x = (float)BLOCK_SIZE * (enemy->GetColumnPos());
      enemy->GetRectangle().y = (float)BLOCK_SIZE * (enemy->GetRowPos());
      return true;
    }
    break;
  }
  case Direction::SOUTH:
  {
    int row = enemy->GetRowPos() + 1;
    int column = enemy->GetColumnPos();
    if (WithinBoundary(row, column) &&
        map[row][column].IsBlock() &&
        CheckCollisionRecs(enemy->GetRectangle(), Rectangle{
                                                      .x = (float)BLOCK_SIZE * column + BLOCK_HITBOX_PADDING,
                                                      .y = (float)BLOCK_SIZE * row + BLOCK_HITBOX_PADDING,
                                                      .width = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2,
                                                      .height = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2}))
    {
      enemy->GetRectangle().x = (float)BLOCK_SIZE * (enemy->GetColumnPos());
      enemy->GetRectangle().y = (float)BLOCK_SIZE * (enemy->GetRowPos());
      return true;
    }
    break;
  }
  case Direction::WEST:
  {
    int row = enemy->GetRowPos();
    int column = enemy->GetColumnPos() - 1;
    if (WithinBoundary(row, column) &&
        map[row][column].IsBlock() &&
        CheckCollisionRecs(enemy->GetRectangle(), Rectangle{
                                                      .x = (float)BLOCK_SIZE * column + BLOCK_HITBOX_PADDING,
                                                      .y = (float)BLOCK_SIZE * row + BLOCK_HITBOX_PADDING,
                                                      .width = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2,
                                                      .height = BLOCK_SIZE - BLOCK_HITBOX_PADDING * 2}))
    {
      enemy->GetRectangle().x = (float)BLOCK_SIZE * (enemy->GetColumnPos());
      enemy->GetRectangle().y = (float)BLOCK_SIZE * (enemy->GetRowPos());

      return true;
    }
    break;
  }
  default:
    break;
  }
  // std::cout << "BOMB\n";
  {
    int adjacentRows[8] = {-1, -1, -1, 0, 0, 1, 1, 1};    // row offsets
    int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1}; // column offsets
    std::lock_guard<std::mutex> lock(bombsMutex);
    for (int i = 0; i < 8; ++i)
    {
      int tempRow = enemy->GetRowPos() + adjacentRows[i];
      int tempColumn = enemy->GetColumnPos() + adjacentColumns[i];

      if (WithinBoundary(tempRow, tempColumn))
      {
        for (auto &bomb : bombsArr)
        {
          if (bomb != nullptr &&
              bomb->HasSamePosition(tempRow, tempColumn) &&
              !bomb->IsIdPresent(id))
          {
            if (CheckCollisionRecs(enemy->GetRectangle(),
                                   Rectangle{
                                       .x = (float)BLOCK_SIZE * tempColumn,
                                       .y = (float)BLOCK_SIZE * tempRow,
                                       .width = BLOCK_SIZE,
                                       .height = BLOCK_SIZE}))
            { // TODO: Maybe change this collision checking with the bomb function and not here
              enemy->GetRectangle().x = (float)BLOCK_SIZE * (enemy->GetColumnPos());
              enemy->GetRectangle().y = (float)BLOCK_SIZE * (enemy->GetRowPos());
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

bool Map::IsEnemyCollidingHelper(size_t row, size_t column)
{
  if (map[row][column].IsBlock())
  {
    return true;
  }
  return false;
}

bool Map::CollidingWithEnemy(int id)
{
  if (players.find(id) == players.end())
    return false;

  std::shared_ptr<Player> player = players.at(id);

  if (!player->IsAlive())
    return false;

  for (auto it = enemies.begin(); it != enemies.end(); ++it)
  {
    if (it->second->IsAlive() && CheckCollisionRecs(player->GetRectangle(), it->second->GetRectangle()))
    {
      return true;
    }
  }
  return false;
}

void Map::CollidingWithPowerUp(int id)
{
  if (players.find(id) == players.end())
    return;

  std::shared_ptr<Player> player = players.at(id);

  if (!player->IsAlive())
    return;

  int adjacentRows[8] = {-1, -1, -1, 0, 0, 1, 1, 1};    // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1}; // column offsets

  for (int i = 0; i < 8; ++i)
  {
    int newRow = player->GetRowPos() + adjacentRows[i];
    int newCol = player->GetColumnPos() + adjacentColumns[i];

    if (WithinBoundary(newRow, newCol))
    {
      if (map[newRow][newCol].powerUp != Power::NO_POWER &&
          map[newRow][newCol].CollidingWithPowerUp(player->GetRectangle()))
      {
        switch (map[newRow][newCol].powerUp)
        {
        case Power::SPEED:
        {
          player->ModifyStat(StatType::Speed, 1.20f);
          map[newRow][newCol].SetPowerType(Power::NO_POWER);
          break;
        }
        case Power::BOMB_RADIUS:
        {
          player->ModifyStat(StatType::BombRadius, 1);
          map[newRow][newCol].SetPowerType(Power::NO_POWER);
          break;
        }
        case Power::BOMB_COUNT:
        {
          player->ModifyStat(StatType::BombCount, 1);
          map[newRow][newCol].SetPowerType(Power::NO_POWER);
          break;
        }
        default:

          break;
        }
      }
    }
  }
}

void Map::RedrawExplodeArea()
{
  for (auto position : explodeArr)
  {
    if (Vector2Equal(map[position.first][position.second].texturePart, MIDDLE))
      continue;

    int top = CheckIfExplosion(position.first - 1, position.second);
    int bottom = CheckIfExplosion(position.first + 1, position.second);
    int left = CheckIfExplosion(position.first, position.second - 1);
    int right = CheckIfExplosion(position.first, position.second + 1);

    int res = top + bottom + left + right;

    if (res == 0)
      continue;

    switch (res)
    {
    case 0:
      continue;
    case 1:
    {
      map[position.first][position.second].texturePart = END;
      if (top == 1)
      {
        map[position.first][position.second].rotation = 180;
      }
      else if (bottom == 1)
      {
        map[position.first][position.second].rotation = 0;
      }
      else if (right == 1)
      {
        map[position.first][position.second].rotation = -90;
      }
      else if (left == 1)
      {
        map[position.first][position.second].rotation = 90;
      }
      break;
    }
    case 2:
    {
      if (top == 1)
      {
        if (bottom == 1)
        {
          map[position.first][position.second].texturePart = PIPE;
          map[position.first][position.second].rotation = 0;
        }
        else if (left == 1)
        {
          map[position.first][position.second].texturePart = TWO_MERGE;
          map[position.first][position.second].rotation = 180;
        }
        else if (right == 1)
        {
          map[position.first][position.second].texturePart = TWO_MERGE;
          map[position.first][position.second].rotation = -90;
        }
      }
      else if (bottom == 1)
      {
        if (left == 1)
        {
          map[position.first][position.second].texturePart = TWO_MERGE;
          map[position.first][position.second].rotation = 90;
        }
        else if (right == 1)
        {
          map[position.first][position.second].texturePart = TWO_MERGE;
          map[position.first][position.second].rotation = 0;
        }
      }
      else if (left == 1 && right == 1)
      {
        map[position.first][position.second].texturePart = PIPE;
        map[position.first][position.second].rotation = 90;
      }
      break;
    }
    case 3:
    {
      map[position.first][position.second].texturePart = THREE_MERGE;
      if (top != 1)
      {
        map[position.first][position.second].rotation = 0;
      }
      else if (bottom != 1)
      {
        map[position.first][position.second].rotation = 180;
      }
      else if (left != 1)
      {
        map[position.first][position.second].rotation = -90;
      }
      else if (right != 1)
      {
        map[position.first][position.second].rotation = 90;
      }
      break;
    }
    case 4:
    {
      map[position.first][position.second].texturePart = FOUR_MERGE;
      map[position.first][position.second].rotation = 0;
      break;
    }
    }

    /*
        if(Vector2Equal(map[position.first][position.second].texturePart, FOUR_MERGE)){
          if(res == 4) continue;

          switch(res){
            case 1:{
              map[position.first][position.second].texturePart = END;
              if(top == 1){
                map[position.first][position.second].rotation = 180;
              }else if(bottom == 1){
                map[position.first][position.second].rotation = 0;
              }else if(right == 1){
                map[position.first][position.second].rotation = -90;
              }else if(left == 1){
                map[position.first][position.second].rotation = 90;
              }
              break;
            }
            case 2:{
              if(top == 1){
                if(bottom == 1){
                  map[position.first][position.second].texturePart = PIPE;
                  map[position.first][position.second].rotation = 0;
                }else if(left == 1){
                  map[position.first][position.second].texturePart = TWO_MERGE;
                  map[position.first][position.second].rotation = 180;
                }else if(right == 1){
                  map[position.first][position.second].texturePart = TWO_MERGE;
                  map[position.first][position.second].rotation = -90;
                }
              }else if(bottom == 1){
                if(left == 1){
                  map[position.first][position.second].texturePart = TWO_MERGE;
                  map[position.first][position.second].rotation = 90;
                }else if(right == 1){
                  map[position.first][position.second].texturePart = TWO_MERGE;
                  map[position.first][position.second].rotation = 0;
                }
              }else if(left == 1 && right == 1){
                map[position.first][position.second].texturePart = PIPE;
                map[position.first][position.second].rotation = 90;
              }
              break;
            }
            case 3:{


              break;
            }
          }


        }else if(Vector2Equal(map[position.first][position.second].texturePart, THREE_MERGE)){

        }else if(Vector2Equal(map[position.first][position.second].texturePart, TWO_MERGE)){

        }else if(Vector2Equal(map[position.first][position.second].texturePart, PIPE)){

        }
        */
  }
}

bool Map::AllEnemiesDied()
{
  if (enemies.empty())
    return false;
  for (auto it = enemies.begin(); it != enemies.end(); ++it)
  {
    if (it->second->IsAlive())
    {
      return false;
    }
  }
  return true;
}

bool Map::IfPlayerDied()
{
  if (!players.at(playerId)->IsAlive())
  {
    return true;
  }
  return false;
}
