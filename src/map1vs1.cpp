
#include "map1vs1.h"

Map1vs1::Map1vs1(Level level) : Map(level)
{
  camera2.target = (Vector2){0, 0}; // Point the camera follows
  camera2.rotation = 0.0f;
  camera2.zoom = 1.0f;
}

Map1vs1::~Map1vs1()
{
}

void Map1vs1::Initialize(std::shared_ptr<Map> mapPtr)
{
  map.resize(numRows);
  for (size_t i = 0; i < numRows; i++)
  {
    map[i].resize(numCols);
  }

  // Load 3D grass models for the 1vs1 arena
  grass[0] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));
  grass[1] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));
  grass[2] = LoadModelFromMesh(GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1));

  grass[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS1);
  grass[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS2);
  grass[2].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS3);
  boundaryModel = getModel(m_BOUNDARY);

  // Position 3D perspective camera over the center of 1vs1 arena
  camera3d = {0};
  camera3d.position = {(float)numCols * BLOCK_SIZE_3D / 2.0f, 25.0f, (float)numRows * BLOCK_SIZE_3D / 2.0f + 14.0f};
  camera3d.target = {(float)numCols * BLOCK_SIZE_3D / 2.0f, 0.0f, (float)numRows * BLOCK_SIZE_3D / 2.0f};
  camera3d.up = {0.0f, 1.0f, 0.0f};
  camera3d.fovy = 45.0f;
  camera3d.projection = CAMERA_PERSPECTIVE;

  // Build 3D map blocks
  for (size_t i = 0; i < numRows; i++)
  {
    for (size_t j = 0; j < numCols; j++)
    {
      int random = GetRandomValue(0, 50);
      if (random < 40)
        map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[0]);
      else if (random < 45)
        map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[1]);
      else
        map[i][j] = Block(i, j, level.map[i][j], Power::NO_POWER, &grass[2]);
    }
  }

  // Assign powerups to blocks
  for (size_t i = 0; i < level.powerUps.size(); i++)
  {
    int row = level.powerUps[i].row;
    int column = level.powerUps[i].column;
    map[row][column].SetPowerType(level.powerUps[i].type);
  }

  // Initialize Player 1
  playerId = GetRandomValue(0, INT_MAX);
  players.insert({playerId, std::make_shared<Player>(mapPtr, playerId, level.players[0].row, level.players[0].column, PLAYER_ONE)});
  players.at(playerId)->SetSpeed(level.speed);
  players.at(playerId)->SetBombCount(level.bombCount);
  players.at(playerId)->SetBombRadius(level.bombRadius);

  // Initialize Player 2
  playerId2 = GetRandomValue(0, INT_MAX);
  players.insert({playerId2, std::make_shared<Player>(mapPtr, playerId2, level.players[1].row, level.players[1].column, PLAYER_TWO)});
  players.at(playerId2)->SetSpeed(level.speed);
  players.at(playerId2)->SetBombCount(level.bombCount);
  players.at(playerId2)->SetBombRadius(level.bombRadius);
  StartBombCountdownThread();
}

void Map1vs1::HandleInput()
{
  if (players.size() != 2)
    return;

  players.at(playerId)->HandleInput();
  players.at(playerId2)->HandleInput();

  for (auto it = enemies.begin(); it != enemies.end(); ++it)
  {
    if (it->second->IsAlive())
    {
      it->second->Move();
    }
  }
}

bool Map1vs1::IfPlayerDied()
{
  if (!players.at(playerId)->IsAlive() || !players.at(playerId2)->IsAlive())
  {
    return true;
  }
  return false;
}

void Map1vs1::MoveCamera()
{
  {
    Rectangle rect = players.at(playerId)->GetRectangle();

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

  // // Zoom in and out with mouse wheel
  // float zoomFactor = GetMouseWheelMove();
  // if (zoomFactor != 0) {
  //     camera.zoom += zoomFactor * 0.05f;
  // }
  // // Prevent zoom from going too small or too big
  // if (camera.zoom < 0.5f) camera.zoom = 0.5f;

  // if (camera.zoom > 3.0f) camera.zoom = 3.0f;

  // float visibleWidth  = width / camera.zoom;
  // float visibleHeight = height / camera.zoom;
  // if(visibleWidth > mapWidth + MAP_BORDER*2){ // If can see entire map then camera target is middle of map
  //   camera.target.x = (float)(mapWidth)/2;
  // }
  // if(visibleHeight > mapHeight + MAP_BORDER*2){
  //   camera.target.y = (float)(mapHeight)/2;
  // }
  {

    Rectangle rect = players.at(playerId2)->GetRectangle();

    int centerX = rect.x + rect.width / 2;
    int centerY = rect.y + rect.height / 2;

    int width = GetScreenWidth();
    int height = GetScreenHeight();

    camera2.offset = Vector2{(float)(width / 2), (float)(height / 2)};

    float halfViewWidth = (width / 2.0f) / camera2.zoom;   // Scale based on zoom
    float halfViewHeight = (height / 2.0f) / camera2.zoom; // Scale based on zoom

    if (centerX + halfViewWidth > mapWidth)
    {
      camera2.target.x = mapWidth - halfViewWidth + MAP_BORDER;
    }
    else if (centerX - halfViewWidth < 0)
    {
      camera2.target.x = halfViewWidth - MAP_BORDER;
    }
    else
    {
      camera2.target.x = centerX;
    }

    if (centerY + halfViewHeight > mapHeight)
    {
      camera2.target.y = mapHeight - halfViewHeight + MAP_BORDER;
    }
    else if (centerY - halfViewHeight < 0)
    {
      camera2.target.y = halfViewHeight - MAP_BORDER;
    }
    else
    {
      camera2.target.y = centerY;
    }

    float visibleWidth = width / camera2.zoom;
    float visibleHeight = height / camera2.zoom;
    if (visibleWidth > mapWidth + MAP_BORDER * 2)
    { // If can see entire map then camera target is middle of map
      camera2.target.x = (float)(mapWidth) / 2;
    }
    if (visibleHeight > mapHeight + MAP_BORDER * 2)
    {
      camera2.target.y = (float)(mapHeight) / 2;
    }
  }
}

void Map1vs1::Draw()
{
  BeginMode3D(camera3d);

  // 1. Draw 3D Arena Boundary
  DrawMapBoundary(numCols, numRows);

  // 2. Draw 3D Blocks (grass, destructible clay, hard bricks)
  for (size_t row = 0; row < numRows; row++)
  {
    for (size_t column = 0; column < numCols; column++)
    {
      map[row][column].Draw3d();
    }
  }

  // 3. Draw 3D Bombs
  {
    std::lock_guard<std::mutex> lock(bombsMutex);
    for (auto &bomb : bombsArr)
    {
      if (bomb != nullptr)
      {
        bomb->Draw3d();
      }
    }
  }

  // 4. Draw Player 1 & Player 2 in 3D
  if (players.find(playerId) != players.end() && players.at(playerId)->IsAlive())
  {
    players.at(playerId)->Draw3d();
  }
  if (players.find(playerId2) != players.end() && players.at(playerId2)->IsAlive())
  {
    players.at(playerId2)->Draw3d();
  }
  EndMode3D();

  // 5. Render 2D Player Controls / HUD Overlay
  DrawText("P1: WASD + Space", 20, 20, 20, RED);
  DrawText("P2: Arrows + Enter", GetScreenWidth() - 250, 20, 20, BLUE);
}
