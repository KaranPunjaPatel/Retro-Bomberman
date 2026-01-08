
#include "map1vs1.h"

Map1vs1::Map1vs1(Level level) : Map(level)
{
  camera2.target = (Vector2){ 0, 0 };   // Point the camera follows
  camera2.rotation = 0.0f;
  camera2.zoom = 1.0f;
}

Map1vs1::~Map1vs1()
{
}

void Map1vs1::Initialize(std::shared_ptr<Map> mapPtr)
{
  // std::cout << "Other map\n";
  map.resize(numRows);
  for (size_t i = 0; i < numRows; i++)
  {
    map[i].resize(numCols);
  }
    
  // Assign each block of map its type
  for (size_t i = 0; i < numRows; i++)
  {
    for (size_t j = 0; j < numCols; j++)
    {
      // std::cout << level.map[i][j] << " " ;
      map[i][j] = Block(i,j, level.map[i][j], Power::NO_POWER);
    }
  }

  // Assign powerups to blocks
  for (size_t i = 0; i < level.powerUps.size(); i++)
  {
    int row    = level.powerUps[i].row;
    int column = level.powerUps[i].column;

    map[row][column].SetPowerType(level.powerUps[i].type); 
  }

  playerId = GetRandomValue(0, INT_MAX);

  players.insert({playerId, std::make_shared<Player>(mapPtr, playerId, level.players[0].row, level.players[0].column)});
  
  players.at(playerId)->speed      = level.speed;
  players.at(playerId)->bombCount  = level.bombCount;
  players.at(playerId)->bombRadius = level.bombRadius;

  playerId2 = GetRandomValue(0, INT_MAX);

  players.insert({playerId2, std::make_shared<Player>(mapPtr, playerId2, level.players[1].row, level.players[1].column)});
  
  players.at(playerId2)->speed      = level.speed;
  players.at(playerId2)->bombCount  = level.bombCount;
  players.at(playerId2)->bombRadius = level.bombRadius;

  StartBombCountdownThread();
}

void Map1vs1::HandleInput()
{
  if(players.size() != 2) return;
  
  players.at(playerId)->HandleInput();
  players.at(playerId2)->HandleInput2();
  

  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(it->second->alive){
      it->second->Move();
    }
  }
}

bool Map1vs1::IfPlayerDied()
{
  if(!players.at(playerId)->alive || !players.at(playerId2)->alive){
    return true;
  }
  return false;
}

void Map1vs1::MoveCamera()
{
  {
    Rectangle rect = players.at(playerId)->rect;

    int centerX = rect.x + rect.width/2;
    int centerY = rect.y + rect.height/2;

    int width  = GetScreenWidth();
    int height = GetScreenHeight();

    camera.offset = Vector2{(float)(width/2), (float)(height/2)};

    float halfViewWidth  = (width / 2.0f) / camera.zoom;  // Scale based on zoom
    float halfViewHeight = (height / 2.0f) / camera.zoom; // Scale based on zoom

    if (centerX + halfViewWidth > mapWidth) {
        camera.target.x = mapWidth - halfViewWidth + MAP_BORDER;
    } else if (centerX - halfViewWidth < 0) {
        camera.target.x = halfViewWidth - MAP_BORDER;
    } else {
        camera.target.x = centerX;
    }

    if (centerY + halfViewHeight > mapHeight) {
        camera.target.y = mapHeight - halfViewHeight + MAP_BORDER;
    } else if (centerY - halfViewHeight < 0) {
        camera.target.y = halfViewHeight - MAP_BORDER;
    } else {
        camera.target.y = centerY;
    }

    float visibleWidth  = width / camera.zoom;
    float visibleHeight = height / camera.zoom;
    if(visibleWidth > mapWidth + MAP_BORDER*2){ // If can see entire map then camera target is middle of map
      camera.target.x = (float)(mapWidth)/2; 
    }
    if(visibleHeight > mapHeight + MAP_BORDER*2){
      camera.target.y = (float)(mapHeight)/2;
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

    Rectangle rect = players.at(playerId2)->rect;
    
    int centerX = rect.x + rect.width/2;
    int centerY = rect.y + rect.height/2;
    
    int width  = GetScreenWidth();
    int height = GetScreenHeight();

    camera2.offset = Vector2{(float)(width/2), (float)(height/2)};

    float halfViewWidth  = (width / 2.0f) / camera2.zoom;  // Scale based on zoom
    float halfViewHeight = (height / 2.0f) / camera2.zoom; // Scale based on zoom
    
    if (centerX + halfViewWidth > mapWidth) {
        camera2.target.x = mapWidth - halfViewWidth + MAP_BORDER;
      } else if (centerX - halfViewWidth < 0) {
        camera2.target.x = halfViewWidth - MAP_BORDER;
      } else {
        camera2.target.x = centerX;
    }
    
    if (centerY + halfViewHeight > mapHeight) {
        camera2.target.y = mapHeight - halfViewHeight + MAP_BORDER;
    } else if (centerY - halfViewHeight < 0) {
      camera2.target.y = halfViewHeight - MAP_BORDER;
    } else {
      camera2.target.y = centerY;
    }

    float visibleWidth  = width / camera2.zoom;
    float visibleHeight = height / camera2.zoom;
    if(visibleWidth > mapWidth + MAP_BORDER*2){ // If can see entire map then camera target is middle of map
      camera2.target.x = (float)(mapWidth)/2; 
    }
    if(visibleHeight > mapHeight + MAP_BORDER*2){
      camera2.target.y = (float)(mapHeight)/2;
    }
  }
}

void Map1vs1::Draw() {
    // Get screen dimensions
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    if(camera.target.x == (float)(mapWidth)/2 && camera2.target.y == (float)(mapHeight)/2){
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
        for(auto& bomb : bombsArr){
          if(bomb != nullptr){
            bomb->Draw();
          }
        }
      }

      for (auto it = players.begin(); it != players.end(); ++it) {
        if(it->second->alive){
          it->second->Draw();
        }
      }
      for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if(it->second->alive){
          it->second->Draw();
        }
      }

      EndMode2D();
    }else{
      // Left viewport (Player 1)
    BeginScissorMode(0, 0, GetScreenWidth()/2, GetScreenHeight());
    BeginMode2D(camera);
    {
        // Draw map for first viewport
        for (size_t row = 0; row < numRows; row++) {
            for (size_t column = 0; column < numCols; column++) {
                map[row][column].Draw();
            }
        }

        // Draw bombs in first viewport
        {
            std::lock_guard<std::mutex> lock(bombsMutex);
            for(auto& bomb : bombsArr) {
                if(bomb != nullptr) {
                    bomb->Draw();
                }
            }
        }

        // Draw enemies in first viewport
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if(it->second->alive) {
                it->second->Draw();
            }
        }

        // Draw player 1
        players.at(playerId)->Draw();
        players.at(playerId2)->Draw();

    }
    EndMode2D();
    EndScissorMode();
    // Right viewport (Player 2)
    BeginScissorMode(GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight());
    BeginMode2D(camera2);
    {
        // Draw map for second viewport
        for (size_t row = 0; row < numRows; row++) {
            for (size_t column = 0; column < numCols; column++) {
                map[row][column].Draw();
            }
        }

        // Draw bombs in second viewport
        {
            std::lock_guard<std::mutex> lock(bombsMutex);
            for(auto& bomb : bombsArr) {
                if(bomb != nullptr) {
                    bomb->Draw();
                }
            }
        }

        // Draw enemies in second viewport
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if(it->second->alive) {
                it->second->Draw();
            }
        }

        // Draw player 2
        players.at(playerId)->Draw();
        players.at(playerId2)->Draw();
    }
    EndMode2D();
    EndScissorMode();

    // Draw viewport separator
    DrawLine(screenWidth/2, 0, screenWidth/2, screenHeight, BLACK);
    }

    
}
