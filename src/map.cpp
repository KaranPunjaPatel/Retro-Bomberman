#include "map.h"
#include "macros.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include "util.h"


Map::Map(Level level) : level(level), stopFlag(false), explodeBomb(false), camera({0})
{
  numRows  = level.rows;
  numCols  = level.columns;
  cellSize = BLOCK_SIZE;

  mapWidth  = cellSize * numCols;
  mapHeight = cellSize * numRows;

  startTime = GetTime();
  finishTime = 0.0;

  camera.target = (Vector2){ 0, 0 };   // Point the camera follows
  // camera.offset = (Vector2){ screenWidth / 2, screenHeight / 2 }; // Camera center
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  // camera3d.

  camera3d = { 0 };
	camera3d.position = { 0.0f, 20.0f, 10.f };
	camera3d.target = { 0.0f, 0.0f, 0.0f };
	camera3d.up = { 0.0f, 1.0f, 0.0f };
	camera3d.fovy = 45.0f;
	camera3d.projection = CAMERA_PERSPECTIVE;

  Mesh mesh = GenMeshPlane(BLOCK_SIZE_3D, BLOCK_SIZE_3D, 1, 1);
  
  Material material1 = LoadMaterialDefault();
  Material material2 = LoadMaterialDefault();
  Material material3 = LoadMaterialDefault();

  material1.maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS1);
  material2.maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS2);
  material3.maps[MATERIAL_MAP_DIFFUSE].texture = getTexture(t_GRASS3);

  grass[0] = LoadModelFromMesh(mesh);
  grass[1] = LoadModelFromMesh(mesh);
  grass[2] = LoadModelFromMesh(mesh);

  grass[0].materials[0] = material1;
  grass[1].materials[0] = material2;
  grass[2].materials[0] = material3;

  boundaryModel = getModel(m_BOUNDARY);

}

Map::~Map(){
  stopFlag.store(true, std::memory_order_release);

  
}

void Map::StartBombCountdownThread()
{
  bombCountdown = std::thread([this]{ // Thread runs to countdown the bomb timer's
      while (!stopFlag.load(std::memory_order_acquire))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        std::lock_guard<std::mutex> lock(bombsMutex);
        
        if (!bombsArr.empty()) {
          for (auto& bomb : bombsArr) {
            if (bomb != nullptr && bomb->CountDown()) {
              explodeBomb.store(true);  // Set the explosion flag if bomb countdown reaches zero
            }
          }
        }
      }
    });

  bombCountdown.detach();
}

void Map::Initialize(std::shared_ptr<Map> mapPtr)
{
  // std::cout << "New Map\n";
  // Create the map according to size
  map.resize(numRows);
  for (size_t i = 0; i < numRows; i++)
  {
    map[i].resize(numCols);
  }
    
  // Assign each block of map its type
  if(SHOW_2D == 1){
    for (size_t i = 0; i < numRows; i++)
    {
      for (size_t j = 0; j < numCols; j++)
      {
        
        map[i][j] = Block(i,j, level.map[i][j], Power::NO_POWER);
      }
    }
  }else{
    // loadAnimations();
    // camera3d.position = BLOCK_SIZE_3D
    BoundingBox box = GetModelBoundingBox(*boundaryModel);

    float modelHeight = box.max.y - box.min.y;

    if(modelHeight < BLOCK_SIZE_3D)
    {
      Matrix scaleMatrix = MatrixScale(1.0f, BLOCK_SIZE_3D / modelHeight, 1.0f);
      (*boundaryModel).transform = MatrixMultiply((*boundaryModel).transform, scaleMatrix);
    }

    for (size_t i = 0; i < numRows; i++)
    {
      for (size_t j = 0; j < numCols; j++)
      {
        int random = GetRandomValue(0,50);
        if(random < 40){
          map[i][j] = Block(i,j, level.map[i][j], Power::NO_POWER, &grass[0]);
        }else if(random < 45){
          map[i][j] = Block(i,j, level.map[i][j], Power::NO_POWER, &grass[1]);
        }else{
          map[i][j] = Block(i,j, level.map[i][j], Power::NO_POWER, &grass[2]);
        }
      }
    }
  }

  // Assign powerups to blocks
  for (size_t i = 0; i < level.powerUps.size(); i++)
  {
    int row    = level.powerUps[i].row;
    int column = level.powerUps[i].column;

    map[row][column].SetPowerType(level.powerUps[i].type); 
  }

  // players.resize(1);
  playerId = GetRandomValue(0, INT_MAX);

  players.insert({playerId, std::make_shared<Player>(mapPtr, playerId, 0,0)});

  players.at(playerId)->speed      = level.speed;
  players.at(playerId)->bombCount  = level.bombCount;
  players.at(playerId)->bombRadius = level.bombRadius;

  for(auto pos : level.enemies){
    int id = GetRandomValue(0, INT_MAX);
    std::cout << pos.row << " " << pos.column << " " << id << std::endl;
    enemies.insert({id, std::make_shared<Enemy>(mapPtr, id, pos.row, pos.column)});
    enemies.at(id)->speed      = level.speed;
    enemies.at(id)->direction  = pos.direction;
  }

  StartBombCountdownThread();
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
  
if(SHOW_2D == 1){

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

  BeginMode3D(camera3d);

  DrawMapBoundary(numCols, numRows);

  for (size_t row = 0; row < numRows; row++)
  {
    for (size_t column = 0; column < numCols; column++)
    {
      map[row][column].Draw3d();
    }
  }

  {
    std::lock_guard<std::mutex> lock(bombsMutex);
    for(auto& bomb : bombsArr){
      if(bomb != nullptr){
        bomb->Draw3d();
      }
    }
  }

  for (auto it = players.begin(); it != players.end(); ++it) {
    if(it->second->alive){
      it->second->Draw3d();
    }
  }
  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(it->second->alive){
      it->second->Draw3d();
    }
  }
    
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
                      0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                    }, 
                  Rectangle{
                      UI_OFFSET, 
                      UI_OFFSET, 
                      UI_TEXTURE_SIZE, 
                      UI_TEXTURE_SIZE
                    },
                  Vector2{0,0},
                  0.0f, WHITE
                );

    DrawText(
        std::to_string(player->bombCount).c_str(), 
        UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET, 
        UI_OFFSET, 
        UI_FONT_SIZE, 
        BLACK
      );

    int tWidth = MeasureText(std::to_string(player->bombCount).c_str(), UI_FONT_SIZE);

    DrawTexturePro(getTexture(t_RADIUS), 
                  Rectangle{
                      0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                    }, 
                  Rectangle{
                      UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth + UI_OFFSET, 
                      UI_OFFSET, 
                      UI_TEXTURE_SIZE, 
                      UI_TEXTURE_SIZE
                    },
                  Vector2{0,0},
                  0.0f, WHITE
                );

    DrawText(
        std::to_string(player->bombRadius).c_str(), 
        UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET, 
        UI_OFFSET, 
        UI_FONT_SIZE, 
        BLACK
      );
  }
  {
    int screenWidth = GetScreenWidth();
    int tWidth = MeasureText(std::to_string(player->lifeCount).c_str(), UI_FONT_SIZE);

    DrawText(
        std::to_string(player->lifeCount).c_str(), 
        screenWidth - (UI_OFFSET + tWidth), 
        UI_OFFSET, 
        UI_FONT_SIZE, 
        BLACK
      );

    DrawTexturePro(getTexture(t_HEART), 
                  Rectangle{
                        0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                      }, 
                  Rectangle{
                    screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE), 
                    UI_OFFSET, 
                    UI_TEXTURE_SIZE, 
                    UI_TEXTURE_SIZE
                  },
                  Vector2{0,0},
                  0.0f, WHITE
                );

    char time[6];
    if(finishTime != 0.0){
      getTime(finishTime - startTime, time);
    }else{
      getTime(GetTime() - startTime, time);
    }
    
    int tWidth2 = MeasureText(time, UI_FONT_SIZE);

    DrawText(
        time, 
        screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth2), 
        UI_OFFSET, 
        UI_FONT_SIZE, 
        BLACK
      );

    DrawTexturePro(getTexture(t_CLOCK), 
                  Rectangle{
                        0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                      }, 
                  Rectangle{
                    screenWidth - (UI_OFFSET + tWidth + UI_OFFSET + UI_TEXTURE_SIZE + UI_OFFSET + tWidth2 + UI_OFFSET + UI_TEXTURE_SIZE), 
                    UI_OFFSET, 
                    UI_TEXTURE_SIZE, 
                    UI_TEXTURE_SIZE
                  },
                  Vector2{0,0},
                  0.0f, WHITE
                );
  }
}

void Map::DrawLevelEndUI(const char* text)
{
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  Rectangle rect = {
      screenWidth/2 - (screenWidth * .5f)/2, 
      screenHeight/2 - (screenHeight * .5f)/2,
      screenWidth * .5f, 
      screenHeight * .5f
    };
  
  DrawRectangleRoundedLinesEx(rect, 0.5f, 2, 5.0f, SKYBLUE);
  DrawRectangleRounded(rect, 0.5f, 2, BLUE);

  int tWidth = MeasureText(text, UI_FONT_SIZE);

  DrawText(
      text, 
      screenWidth/2 - tWidth/2, 
      rect.y + UI_MARGIN, 
      UI_FONT_SIZE, 
      WHITE
    );

  

  DrawTexturePro(getTexture(t_CLOCK), 
                  Rectangle{
                        0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                      }, 
                  Rectangle{
                    rect.x + UI_OFFSET + UI_MARGIN, 
                    rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET, 
                    UI_TEXTURE_SIZE, 
                    UI_TEXTURE_SIZE
                  },
                  Vector2{0,0},
                  0.0f, WHITE
                );

  DrawText(
      "Time :" , 
      rect.x + UI_OFFSET + UI_MARGIN + UI_TEXTURE_SIZE + UI_OFFSET, 
      rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET,
      UI_FONT_SIZE,
      WHITE
    );

  if(finishTime == 0.0f){
    finishTime = GetTime();
  }

  char time[6];
  getTime(finishTime - startTime, time);

  DrawText(
      time, 
      screenWidth/2 + UI_MARGIN, 
      rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET, 
      UI_FONT_SIZE, 
      WHITE 
    );

  DrawTexturePro(getTexture(t_HEART), 
                  Rectangle{
                        0, 0, UI_TEXTURE_SIZE, UI_TEXTURE_SIZE
                      }, 
                  Rectangle{
                    rect.x + UI_OFFSET + UI_MARGIN, 
                    rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET + UI_FONT_SIZE + UI_OFFSET, 
                    UI_TEXTURE_SIZE, 
                    UI_TEXTURE_SIZE
                  },
                  Vector2{0,0},
                  0.0f, WHITE
                );

  DrawText(
      "Lives left :" , 
      rect.x + UI_OFFSET + UI_MARGIN + UI_TEXTURE_SIZE + UI_OFFSET, 
      rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET + UI_FONT_SIZE + UI_OFFSET,
      UI_FONT_SIZE,
      WHITE
    );

  DrawText(
      std::to_string(players.at(playerId)->lifeCount).c_str(), 
      screenWidth/2 + UI_MARGIN, 
      rect.y + UI_MARGIN + UI_FONT_SIZE + UI_OFFSET + UI_FONT_SIZE + UI_OFFSET, 
      UI_FONT_SIZE, 
      WHITE 
    );
}

void Map::DrawMapBoundary(float width, float length)
{
  BoundingBox box = GetModelBoundingBox(*boundaryModel);

  float modelWidth = box.max.x - box.min.x;
  float modelLength = box.max.z - box.min.z;

  float tempwidth = 0;

  DrawModel(*boundaryModel, Vector3{-modelWidth/2, 0, -modelLength/2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{-modelWidth/2, 0, length+modelLength/2}, 1, WHITE);

  while(tempwidth <= width)
  {
      tempwidth += modelWidth/1.5;
      DrawModel(*boundaryModel, Vector3{tempwidth - modelWidth/2, 0, -modelLength/2}, 1, WHITE);

      DrawModel(*boundaryModel, Vector3{tempwidth - modelWidth/2, 0, length+modelLength/2}, 1, WHITE);
  }
  DrawModel(*boundaryModel, Vector3{width + modelWidth/2, 0, -modelLength/2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{width + modelWidth/2, 0, length+modelLength/2}, 1, WHITE);


  float templength = 0;

  while(templength <= length)
  {
      templength += modelLength/1.5;
      DrawModel(*boundaryModel, Vector3{-modelWidth/2, 0, templength-modelLength/2}, 1, WHITE);
      DrawModel(*boundaryModel, Vector3{width+modelWidth/2, 0, templength-modelLength/2}, 1, WHITE);
  }
  DrawModel(*boundaryModel, Vector3{-modelWidth/2, 0, length+modelLength/2}, 1, WHITE);
  DrawModel(*boundaryModel, Vector3{width+modelWidth/2, 0, length+modelLength/2}, 1, WHITE);
}

void Map::MoveCamera()
{
  Rectangle rect = players.at(playerId)->rect;
  if(SHOW_2D == 1){

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

    // Zoom in and out with mouse wheel
    float zoomFactor = GetMouseWheelMove();
    if (zoomFactor != 0) {
        camera.zoom += zoomFactor * 0.05f;
        camera3d.fovy += zoomFactor * 2.0f;
        // std::cout << "Change\n";
    }
    // Prevent zoom from going too small or too big
    if (camera.zoom < 0.5f) camera.zoom = 0.5f;

    if (camera.zoom > 3.0f) camera.zoom = 3.0f;

    float visibleWidth  = width / camera.zoom;
    float visibleHeight = height / camera.zoom;
    if(visibleWidth > mapWidth + MAP_BORDER*2){ // If can see entire map then camera target is middle of map
      camera.target.x = (float)(mapWidth)/2; 
    }
    if(visibleHeight > mapHeight + MAP_BORDER*2){
      camera.target.y = (float)(mapHeight)/2;
    }
  }
  else
  {
    camera3d.target = {rect.x/BLOCK_SIZE, 0.5, rect.y/BLOCK_SIZE};
    camera3d.position = {rect.x/BLOCK_SIZE, 17, rect.y/BLOCK_SIZE + 7};
  }
}

bool Map::IsColliding(int id)
{
  if(players.find(id) == players.end()) return true;

  Rectangle* rect = &players.at(id)->rect;
  int* row = &players.at(id)->rowPos;
  int* column = &players.at(id)->colPos;

  int adjacentRows[8]    = {-1, -1, -1, 0, 0, 1, 1, 1};  // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};  // column offsets

  for (size_t i = 0; i < 8; ++i) {
    int tempRow = *row + adjacentRows[i];
    int tempColumn = *column + adjacentColumns[i];

    if(WithinBoundary(tempRow, tempColumn))
    {
      if(map[tempRow][tempColumn].IsBlock()){
        if(CheckCollisionRecs(*rect, Rectangle{.x = (float)BLOCK_SIZE * tempColumn, .y = (float)BLOCK_SIZE * tempRow, .width = BLOCK_SIZE, .height = BLOCK_SIZE})){
          return true;
        }
      }
    }
  }

  {
    std::lock_guard<std::mutex> lock(bombsMutex);
    for (int i = 0; i < 8; ++i) {
      int tempRow = *row + adjacentRows[i];
      int tempColumn = *column + adjacentColumns[i];

      if(WithinBoundary(tempRow, tempColumn))
      {
        for(auto& bomb : bombsArr){
          if(bomb != nullptr && 
          bomb->HasSamePosition(tempRow, tempColumn) &&
          !bomb->IsIdPresent(id)){
            if(CheckCollisionRecs(*rect, 
                                  Rectangle{
                                    .x = (float)BLOCK_SIZE * tempColumn, 
                                    .y = (float)BLOCK_SIZE * tempRow, 
                                    .width = BLOCK_SIZE, 
                                    .height = BLOCK_SIZE
                                  }))
            { // TODO: Maybe change this collision checking with the bomb function and not here 
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

bool Map::WithinBoundary(size_t row, size_t column) { // checking the boundry
  return (row >= 0 && row < numRows && column >= 0 && column < numCols);
}

void Map::ChangeBombIds(int id){
  std::shared_ptr<Human> human;

  if(players.find(id) != players.end()){
    human = players.at(id);
  }else if(enemies.find(id) != enemies.end()){
    human = enemies.at(id);
  }else{
    return;
  }

  int adjacentRows[8]    = {-1, -1, -1, 0, 0, 1, 1, 1};  // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};  // column offsets

  for (int i = 0; i < 8; ++i) { // Changes the bool for bombs to see if player can pass through it or not
    int newRow = human->rowPos + adjacentRows[i];
    int newCol = human->colPos + adjacentColumns[i];

    if(WithinBoundary(newRow, newCol))
    {
      std::lock_guard<std::mutex> lock(bombsMutex);
      for(auto& bomb : bombsArr){
        // std::cout << "Id: " << id <<" "<< bomb->HasSamePosition(newRow, newCol) <<" "<< bomb->IsIdPresent(id) << std::endl;

        if(bomb != nullptr && 
        bomb->HasSamePosition(newRow, newCol) && 
        bomb->IsIdPresent(id)){
          if(!CheckCollisionRecs(human->rect, 
                                  Rectangle{
                                    .x = (float)BLOCK_SIZE * newCol, 
                                    .y = (float)BLOCK_SIZE * newRow, 
                                    .width = BLOCK_SIZE, 
                                    .height = BLOCK_SIZE
                                  }))
          {
            // std::cout << "Remove: " << id << std::endl;
            bomb->ids.erase(std::remove(bomb->ids.begin(), bomb->ids.end(), id), bomb->ids.end());
          }
        }
      }
    }
  }
}

void Map::HandleInput()
{
  // for (auto it = players.begin(); it != players.end(); ++it) {
  //   it->second->HandleInput();
  // }
  if(players.at(playerId)->alive)
    players.at(playerId)->HandleInput();

  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(it->second->alive){
      it->second->Move();
    }
  }
}

bool Map::AlreadyHasBomb(size_t row, size_t column)
{
  {
    std::lock_guard<std::mutex> lock(bombsMutex);
    for(auto& bomb : bombsArr){
      if(bomb != nullptr && bomb->HasSamePosition(row, column)){
        return true;
      }
    }
  }
  return false;
}

void Map::AddBomb(int id)
{
  if(players.find(id) == players.end()) return;

  std::shared_ptr<Player> player = players.at(id);

  std::vector<int> ids;

  /*
  Vector2 circleCenter = {(float)BLOCK_SIZE * column + BLOCK_SIZE/2, 
                                  (float)BLOCK_SIZE * row + BLOCK_SIZE/2};
  float radius = (float)BLOCK_SIZE/2.5;
*/
  
  Rectangle rect = {
    (float)(BLOCK_SIZE * player->colPos), (float)(BLOCK_SIZE * player->rowPos), (float)BLOCK_SIZE, (float)BLOCK_SIZE
  };

  for (auto it = players.begin(); it != players.end(); ++it) {
    if(CheckCollisionRecs(rect, it->second->rect))
    {
      ids.push_back(it->first);
    }
  }

  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(CheckCollisionRecs(rect, it->second->rect))
    {
      ids.push_back(it->first);
    }
  }

  bombsArr.push_back(std::make_shared<Bomb>(id,player->rowPos,player->colPos,player->bombRadius,ids));
}

void Map::ExplodeBomb()
{
  if(explodeBomb.load()){
    std::lock_guard<std::mutex> lock(bombsMutex);

    while(!bombsArr.empty()){

      std::shared_ptr<Bomb> bomb = bombsArr.front();
      // std::cout << (bomb != nullptr) << " " << bomb->blast.load() << std::endl;
      if(bomb != nullptr && bomb->blast.load()){
        // std::cout << "Remove Bomb!!!" << std::endl;
        int id = bomb->owner;
        std::shared_ptr<Player> player = players.at(id); 

        ExplodeArea(bomb);
        RedrawExplodeArea();
        processedBombsArr.push_back(bomb);
        bombsArr.erase(bombsArr.begin());
        player->bombCount++;
      }else{
        break;
      }
    }

    explodeBomb.store(false);
  }

  if(!explodeArr.empty())
  {

    bool res = false;
    // TODO: Clear explode area whose time is up  
    explodeArr.erase(
      std::remove_if(
          explodeArr.begin(),
          explodeArr.end(),
          [&](const std::pair<int, int>& p) {

            if(map[p.first][p.second].ClearExplode()){
              map[p.first][p.second].SetType(GRASS_BLOCK);
              res = true;
              return true;
            }

            return false;
          }
      ),
      explodeArr.end()
    );
    
    if(res){
      RedrawExplodeArea();
    }
  }
}

void Map::ExplodeArea(std::shared_ptr<Bomb> bomb)
{
  int row = bomb->row;
  int column = bomb->column;
  int radius = bomb->bombRadius;

  map[row][column].SetType(EXPLODE);
  map[row][column].SetExploadeTimer();
  explodeArr.push_back(std::make_pair(row, column));
  // map[row][column].texturePart = {.x = 64, .y = 64};
  map[row][column].texturePart = MIDDLE;
  map[row][column].rotation = 0;

  for(int i = 1; i <= radius; i++){ // Top
    int tempRow = row - i;
    int tempCol = column;

    for(auto& bomb : bombsArr){
      if(bomb != nullptr && bomb->HasSamePosition(tempRow, tempCol)){
        bomb->SetToExplode();
      }
    }

    int res = ExplodeAreaHelper(tempRow,tempCol);


    if(res == 0){
      break;
    }else if(res == 1){ 
      // map[tempRow][tempCol].texturePart = {.x = 64, .y = 0};
      
      int further = CheckIfExplosion(tempRow-1, tempCol);
      int side1   = CheckIfExplosion(tempRow, tempCol-1);
      int side2   = CheckIfExplosion(tempRow, tempCol+1);
      
      int sidesExploding = further + side1 + side2;
      
      if(sidesExploding == 0){
        map[tempRow][tempCol].texturePart = END;
        map[tempRow][tempCol].rotation = 0;
      }else if(sidesExploding == 1){
        if(further == 1){
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }else{
          map[tempRow][tempCol].texturePart = TWO_MERGE;
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 90;
          }else{
            map[tempRow][tempCol].rotation = 0;
          }
        }
      }else if(sidesExploding == 2){
        map[tempRow][tempCol].texturePart = THREE_MERGE;
        if(further == 1){
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 90;
          }else{
            map[tempRow][tempCol].rotation = -90;
          }
        }else{
          map[tempRow][tempCol].rotation = 0;
        }
      }else if(sidesExploding == 3){
        map[tempRow][tempCol].texturePart = FOUR_MERGE;
        map[tempRow][tempCol].rotation = 0;
      }
      break;
    }else if(res == 2){
      /*
      if(i == radius){
        if(WithinBoundary(tempRow-1, tempCol) && 
          map[tempRow-1][tempCol].type == EXPLODE
        ){
          map[tempRow][tempCol].texturePart = {.x = 64, .y = 32};
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }else{
          map[tempRow][tempCol].texturePart = {.x = 64, .y = 0};
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 0;
        }
      }else{
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 32};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
      }
      */

      int further = CheckIfExplosion(tempRow-1, tempCol);
      int side1   = CheckIfExplosion(tempRow, tempCol-1);
      int side2   = CheckIfExplosion(tempRow, tempCol+1);
      
      int sidesExploding = further + side1 + side2;

      if(i == radius){
        if(sidesExploding == 0){
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 0;
        }else if(sidesExploding == 1){
          if(further == 1){
            map[tempRow][tempCol].texturePart = PIPE;
            map[tempRow][tempCol].rotation = 0;
          }else{
            map[tempRow][tempCol].texturePart = TWO_MERGE;
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 90;
            }else{
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }else if(sidesExploding == 2){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          if(further == 1){
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 90;
            }else{
              map[tempRow][tempCol].rotation = -90;
            }
          }else{
            map[tempRow][tempCol].rotation = 90;
          }
        }else if(sidesExploding == 3){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }
      }else{
        if(side1 == 1 && side2 == 1){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else if(side1 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 90;
        }else if(side2 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = -90;
        }else{
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }
      }


    }else if(res == 3){
      /*
      if(map[tempRow][tempCol].texturePart.x != 64 && 
      map[tempRow][tempCol].texturePart.y != 64){ // Center of explosion
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 32};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;

      }else if(map[tempRow][tempCol].texturePart.x == 0 || 
      map[tempRow][tempCol].texturePart.y == 0 ||
      map[tempRow][tempCol].texturePart.x == 128 ||
      map[tempRow][tempCol].texturePart.y == 128) // Ends of a explosion
      {
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 32};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
      }
      */

      if(!Vector2Equal(map[tempRow][tempCol].texturePart, MIDDLE) || !Vector2Equal(map[tempRow][tempCol].texturePart, FOUR_MERGE)){
        if(i == radius){
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation != 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }else{
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation != 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180 || map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }
      }
    }
  }

  for(int i = 1; i <= radius; i++){ // Left
    int tempRow = row;
    int tempCol = column - i;

    for(auto& bomb : bombsArr){
      if(bomb != nullptr && bomb->HasSamePosition(tempRow, tempCol)){
        bomb->SetToExplode();
      }
    }

    int res = ExplodeAreaHelper(tempRow,tempCol);
    if(res == 0){
      break;
    }else if(res == 1){
        // map[tempRow][tempCol].texturePart = {.x = 0, .y = 64};
        // map[tempRow][tempCol].texturePart = END;
        // map[tempRow][tempCol].rotation = 270;

      int further = CheckIfExplosion(tempRow, tempCol-1);
      int side1   = CheckIfExplosion(tempRow+1, tempCol);
      int side2   = CheckIfExplosion(tempRow-1, tempCol);
      
      int sidesExploding = further + side1 + side2;

      if(sidesExploding == 0){
        map[tempRow][tempCol].texturePart = END;
        map[tempRow][tempCol].rotation = -90;
      }else if(sidesExploding == 1){
        if(further == 1){
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }else{
          map[tempRow][tempCol].texturePart = TWO_MERGE;
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 0;
          }else{
            map[tempRow][tempCol].rotation = -90;
          }
        }
      }else if(sidesExploding == 2){
        map[tempRow][tempCol].texturePart = THREE_MERGE;
        if(further == 1){
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 0;
          }else{
            map[tempRow][tempCol].rotation = 180;
          }
        }else{
          map[tempRow][tempCol].rotation = -90;
        }
      }else if(sidesExploding == 3){
        map[tempRow][tempCol].texturePart = FOUR_MERGE;
        map[tempRow][tempCol].rotation = 0;
      }
      break;
    }
    else if(res == 2){
      /*
      if(i == radius){
        if(WithinBoundary(tempRow, tempCol-1) && 
          map[tempRow][tempCol-1].type == EXPLODE
        ){
          // std::cout<< "Extra\n";
          map[tempRow][tempCol].texturePart = {.x = 32, .y = 64};
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }else{
          map[tempRow][tempCol].texturePart = {.x = 0, .y = 64};
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = -90;
        }
      }else{
        map[tempRow][tempCol].texturePart = {.x = 32, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
        map[tempRow][tempCol].rotation = 90;
      }
      */

      int further = CheckIfExplosion(tempRow, tempCol-1);
      int side1   = CheckIfExplosion(tempRow+1, tempCol);
      int side2   = CheckIfExplosion(tempRow-1, tempCol);
      
      int sidesExploding = further + side1 + side2;

      if(i == radius){
        if(sidesExploding == 0){
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = -90;
        }else if(sidesExploding == 1){
          if(further == 1){
            map[tempRow][tempCol].texturePart = PIPE;
            map[tempRow][tempCol].rotation = 90;
          }else{
            map[tempRow][tempCol].texturePart = TWO_MERGE;
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 0;
            }else{
              map[tempRow][tempCol].rotation = -90;
            }
          }
        }else if(sidesExploding == 2){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          if(further == 1){
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 0;
            }else{
              map[tempRow][tempCol].rotation = 180;
            }
          }else{
            map[tempRow][tempCol].rotation = -90;
          }
        }else if(sidesExploding == 3){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }
      }else{
        if(side1 == 1 && side2 == 1){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else if(side1 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else if(side2 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 180;
        }else{
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }
      }
    }else if(res == 3){
      /*
      if(map[tempRow][tempCol].texturePart.x != 64 && 
      map[tempRow][tempCol].texturePart.y != 64){
        map[tempRow][tempCol].texturePart = {.x = 32, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
        map[tempRow][tempCol].rotation = 90;
      }else if(map[tempRow][tempCol].texturePart.x == 0 || 
      map[tempRow][tempCol].texturePart.y == 0 ||
      map[tempRow][tempCol].texturePart.x == 128 ||
      map[tempRow][tempCol].texturePart.y == 128)
      {
        map[tempRow][tempCol].texturePart = {.x = 32, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
        map[tempRow][tempCol].rotation = 90;
      }
      */

      if(!Vector2Equal(map[tempRow][tempCol].texturePart, MIDDLE) || !Vector2Equal(map[tempRow][tempCol].texturePart, FOUR_MERGE)){
        if(i == radius){
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }else{
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == 90 || map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }
      }
    }
  }

  for(int i = 1; i <= radius; i++){ // Bottom
    int tempRow = row + i;
    int tempCol = column;

    for(auto& bomb : bombsArr){
      if(bomb != nullptr && bomb->HasSamePosition(tempRow, tempCol)){
        bomb->SetToExplode();
      }
    }

    int res = ExplodeAreaHelper(tempRow,tempCol);
    if(res == 0){
      break;
    }else if(res == 1){
        // map[tempRow][tempCol].texturePart = {.x = 64, .y = 128};
        // map[tempRow][tempCol].texturePart = END;
        // map[tempRow][tempCol].rotation = 180;

      int further = CheckIfExplosion(tempRow+1, tempCol);
      int side1   = CheckIfExplosion(tempRow, tempCol-1);
      int side2   = CheckIfExplosion(tempRow, tempCol+1);
      
      int sidesExploding = further + side1 + side2;

      if(sidesExploding == 0){
        map[tempRow][tempCol].texturePart = END;
        map[tempRow][tempCol].rotation = 180;
      }else if(sidesExploding == 1){
        if(further == 1){
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }else{
          map[tempRow][tempCol].texturePart = TWO_MERGE;
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 180;
          }else{
            map[tempRow][tempCol].rotation = -90;
          }
        }
      }else if(sidesExploding == 2){
        map[tempRow][tempCol].texturePart = THREE_MERGE;
        if(further == 1){
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 90;
          }else{
            map[tempRow][tempCol].rotation = -90;
          }
        }else{
          map[tempRow][tempCol].rotation = 180;
        }
      }else if(sidesExploding == 3){
        map[tempRow][tempCol].texturePart = FOUR_MERGE;
        map[tempRow][tempCol].rotation = 0;
      }
      break;
    }
    else if(res == 2){
      /*
      if(i == radius){
        if(WithinBoundary(tempRow+1, tempCol) && 
          map[tempRow+1][tempCol].type == EXPLODE
        ){
          // std::cout<< "Extra\n";
          map[tempRow][tempCol].texturePart = {.x = 64, .y = 96};
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }else{
          map[tempRow][tempCol].texturePart = {.x = 64, .y = 128};
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 180;

        }
      }else{
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 96};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
      }
      */
      int further = CheckIfExplosion(tempRow+1, tempCol);
      int side1   = CheckIfExplosion(tempRow, tempCol-1);
      int side2   = CheckIfExplosion(tempRow, tempCol+1);
      
      int sidesExploding = further + side1 + side2;

      if(i == radius){
        if(sidesExploding == 0){
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 180;
        }else if(sidesExploding == 1){
          if(further == 1){
            map[tempRow][tempCol].texturePart = PIPE;
            map[tempRow][tempCol].rotation = 0;
          }else{
            map[tempRow][tempCol].texturePart = TWO_MERGE;
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 180;
            }else{
              map[tempRow][tempCol].rotation = -90;
            }
          }
        }else if(sidesExploding == 2){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          if(further == 1){
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 90;
            }else{
              map[tempRow][tempCol].rotation = -90;
            }
          }else{
            map[tempRow][tempCol].rotation = 180;
          }
        }else if(sidesExploding == 3){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }
      }else{
        if(side1 == 1 && side2 == 1){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else if(side1 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 90;
        }else if(side2 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = -90;
        }else{
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
        }
      }

    }else if(res == 3){

      /*
      if(map[tempRow][tempCol].texturePart.x != 64 && 
      map[tempRow][tempCol].texturePart.y != 64){
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 96};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
      }else if(map[tempRow][tempCol].texturePart.x == 0 || 
      map[tempRow][tempCol].texturePart.y == 0 ||
      map[tempRow][tempCol].texturePart.x == 128 ||
      map[tempRow][tempCol].texturePart.y == 128)
      {
        map[tempRow][tempCol].texturePart = {.x = 64, .y = 96};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 0;
      }
      */

      if(!Vector2Equal(map[tempRow][tempCol].texturePart, MIDDLE) || !Vector2Equal(map[tempRow][tempCol].texturePart, FOUR_MERGE)){
        if(i == radius){
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation != 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }else if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }else if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }else{
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation != 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 0 || map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = -90;
            }else if(map[tempRow][tempCol].rotation == 90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }
      }
    }
  }

  for(int i = 1; i <= radius; i++){ // Right
    int tempRow = row;
    int tempCol = column + i;

    for(auto& bomb : bombsArr){
      if(bomb != nullptr && bomb->HasSamePosition(tempRow, tempCol)){
        bomb->SetToExplode();
      }
    }
    
    int res = ExplodeAreaHelper(tempRow,tempCol);
    if(res == 0){
      break;
    }else if(res == 1){
      // map[tempRow][tempCol].texturePart = {.x = 128, .y = 64};
      // map[tempRow][tempCol].texturePart = END;
      // map[tempRow][tempCol].rotation = 90;

      int further = CheckIfExplosion(tempRow, tempCol+1);
      int side1   = CheckIfExplosion(tempRow-1, tempCol);
      int side2   = CheckIfExplosion(tempRow+1, tempCol);
      
      int sidesExploding = further + side1 + side2;

      if(sidesExploding == 0){
        map[tempRow][tempCol].texturePart = END;
        map[tempRow][tempCol].rotation = 90;
      }else if(sidesExploding == 1){
        if(further == 1){
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }else{
          map[tempRow][tempCol].texturePart = TWO_MERGE;
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 180;
          }else{
            map[tempRow][tempCol].rotation = 90;
          }
        }
      }else if(sidesExploding == 2){
        map[tempRow][tempCol].texturePart = THREE_MERGE;
        if(further == 1){
          if(side1 == 1){
            map[tempRow][tempCol].rotation = 180;
          }else{
            map[tempRow][tempCol].rotation = 0;
          }
        }else{
          map[tempRow][tempCol].rotation = 90;
        }
      }else if(sidesExploding == 3){
        map[tempRow][tempCol].texturePart = FOUR_MERGE;
        map[tempRow][tempCol].rotation = 0;
      }
      break;
    }
    else if(res == 2){
      /*
      if(i == radius){
        if(WithinBoundary(tempRow, tempCol+1) && 
            map[tempRow][tempCol+1].type == EXPLODE
        ){
          // std::cout<< "Extra\n";
          map[tempRow][tempCol].texturePart = {.x = 96, .y = 64};
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }else{
          map[tempRow][tempCol].texturePart = {.x = 128, .y = 64};
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 90;
        }
      }else{
        map[tempRow][tempCol].texturePart = {.x = 96, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
      }
      */
    
      int further = CheckIfExplosion(tempRow, tempCol+1);
      int side1   = CheckIfExplosion(tempRow-1, tempCol);
      int side2   = CheckIfExplosion(tempRow+1, tempCol);
      
      int sidesExploding = further + side1 + side2;

      if(i == radius){
        if(sidesExploding == 0){
          map[tempRow][tempCol].texturePart = END;
          map[tempRow][tempCol].rotation = 90;
        }else if(sidesExploding == 1){
          if(further == 1){
            map[tempRow][tempCol].texturePart = PIPE;
            map[tempRow][tempCol].rotation = 90;
          }else{
            map[tempRow][tempCol].texturePart = TWO_MERGE;
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 180;
            }else{
              map[tempRow][tempCol].rotation = 90;
            }
          }
        }else if(sidesExploding == 2){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          if(further == 1){
            if(side1 == 1){
              map[tempRow][tempCol].rotation = 180;
            }else{
              map[tempRow][tempCol].rotation = 0;
            }
          }else{
            map[tempRow][tempCol].rotation = 90;
          }
        }else if(sidesExploding == 3){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }
      }else{
        if(side1 == 1 && side2 == 1){
          map[tempRow][tempCol].texturePart = FOUR_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else if(side1 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 180;
        }else if(side2 == 1){
          map[tempRow][tempCol].texturePart = THREE_MERGE;
          map[tempRow][tempCol].rotation = 0;
        }else{
          map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
        }
      }

    }else if(res == 3){
      /*
      if(map[tempRow][tempCol].texturePart.x != 64 && 
      map[tempRow][tempCol].texturePart.y != 64){
        map[tempRow][tempCol].texturePart = {.x = 96, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
      }else if(map[tempRow][tempCol].texturePart.x == 0 || 
      map[tempRow][tempCol].texturePart.y == 0 ||
      map[tempRow][tempCol].texturePart.x == 128 ||
      map[tempRow][tempCol].texturePart.y == 128)
      {
        map[tempRow][tempCol].texturePart = {.x = 96, .y = 64};
        map[tempRow][tempCol].texturePart = PIPE;
          map[tempRow][tempCol].rotation = 90;
      }
      */
    
      if(!Vector2Equal(map[tempRow][tempCol].texturePart, MIDDLE) || !Vector2Equal(map[tempRow][tempCol].texturePart, FOUR_MERGE)){
        if(i == radius){
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = TWO_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }else{
          if(Vector2Equal(map[tempRow][tempCol].texturePart, PIPE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, END)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == 90 || map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = PIPE;
              map[tempRow][tempCol].rotation = 90;
            }else if(map[tempRow][tempCol].rotation == 180){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, TWO_MERGE)){
            if(map[tempRow][tempCol].rotation == 0){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }else if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = THREE_MERGE;
              map[tempRow][tempCol].rotation = 180;
            }
          }else if(Vector2Equal(map[tempRow][tempCol].texturePart, THREE_MERGE)){
            if(map[tempRow][tempCol].rotation == -90){
              map[tempRow][tempCol].texturePart = FOUR_MERGE;
              map[tempRow][tempCol].rotation = 0;
            }
          }
        }
      }
    }


  }


}

int Map::CheckIfExplosion(size_t row, size_t column) // 0 - Out of boundary / Not exploding, 1 - Exploding
{
  if(WithinBoundary(row,column)){
    if(map[row][column].type == EXPLODE){
      return 1;
    }
  }
  return 0;
}

int Map::ExplodeAreaHelper(size_t row, size_t column) // 0 - Out of boundary, 1 - Clay block broke, 2 - No obsturction, 3 - Was already exploding 
{
  if(WithinBoundary(row, column)){
    if(map[row][column].IsClay()){
      map[row][column].SetType(EXPLODE);
      map[row][column].SetExploadeTimer();
      explodeArr.push_back(std::make_pair(row, column));
      return 1;
    }
    if(map[row][column].type == EXPLODE){
      map[row][column].SetExploadeTimer();
      return 3;
    }
    if(!map[row][column].IsBrick()){
      map[row][column].SetType(EXPLODE);
      map[row][column].SetExploadeTimer();
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

  if(players.find(id) != players.end()){
    human = players.at(id);
  }else if(enemies.find(id) != enemies.end()){
    human = enemies.at(id);
  }else{
    return false;
  }

  if(!human->alive) return false;

  if(map[human->rowPos][human->colPos].type == EXPLODE ){
    return true;
  }

  int adjacentRows[8]    = {-1, -1, -1, 0, 0, 1, 1, 1};  // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};  // column offsets

  for (int i = 0; i < 8; ++i) { 
    int newRow = human->rowPos + adjacentRows[i];
    int newCol = human->colPos + adjacentColumns[i];

    if(WithinBoundary(newRow, newCol)){
      if(map[newRow][newCol].type == EXPLODE && 
      CheckCollisionRecs(human->rect, 
                        Rectangle
                        {(float)(BLOCK_SIZE * newCol),
                        (float)(BLOCK_SIZE * newRow), 
                        (float)BLOCK_SIZE, 
                        (float)BLOCK_SIZE
      })){
        return true;
      }
    }
  }
  return false;
}

bool Map::IsEnemyColliding(int id)
{
  if (enemies.find(id) == enemies.end()) return false;
  // std::cout << id << std::endl;
  std::shared_ptr<Enemy> enemy = enemies.at(id);

  if(enemy->rect.x + enemy->rect.width > mapWidth ||
    enemy->rect.x < 0 ||
    enemy->rect.y + enemy->rect.height > mapHeight ||
    enemy->rect.y < 0)
  {
    enemy->IsTouchingBoundary();
    return true;
  }

  switch (enemy->direction)
  {
  case Direction::NORTH:
  {
    size_t row = enemy->rowPos - 1;
    size_t column = enemy->colPos;
    if(WithinBoundary(row, column) &&
      map[row][column].IsBlock() && 
      CheckCollisionRecs(enemy->rect, Rectangle{
        .x = (float)BLOCK_SIZE * column, 
        .y = (float)BLOCK_SIZE * row, 
        .width = BLOCK_SIZE, 
        .height = BLOCK_SIZE
      })
    ){
      enemy->rect.x = (float)BLOCK_SIZE * (enemy->colPos);
      enemy->rect.y = (float)BLOCK_SIZE * (enemy->rowPos);
      return true;
    }
    break;
  }
  case Direction::EAST:
  {
    size_t row = enemy->rowPos;
    size_t column = enemy->colPos + 1;
    if(WithinBoundary(row, column) &&
      map[row][column].IsBlock() && 
      CheckCollisionRecs(enemy->rect, Rectangle{
        .x = (float)BLOCK_SIZE * column, 
        .y = (float)BLOCK_SIZE * row, 
        .width = BLOCK_SIZE, 
        .height = BLOCK_SIZE
      })
    ){
      enemy->rect.x = (float)BLOCK_SIZE * (enemy->colPos);
      enemy->rect.y = (float)BLOCK_SIZE * (enemy->rowPos);
      return true;
    }
    break;
  }
  case Direction::SOUTH:
  {
    size_t row = enemy->rowPos + 1;
    size_t column = enemy->colPos;
    if(WithinBoundary(row, column) &&
      map[row][column].IsBlock() && 
      CheckCollisionRecs(enemy->rect, Rectangle{
        .x = (float)BLOCK_SIZE * column, 
        .y = (float)BLOCK_SIZE * row,  
        .width = BLOCK_SIZE, 
        .height = BLOCK_SIZE
      })
    ){
      enemy->rect.x = (float)BLOCK_SIZE * (enemy->colPos);
      enemy->rect.y = (float)BLOCK_SIZE * (enemy->rowPos);
      return true;
    }
    break;
  }
  case Direction::WEST:
  {
    size_t row = enemy->rowPos;
    size_t column = enemy->colPos - 1;
    if(WithinBoundary(row, column) &&
      map[row][column].IsBlock() && 
      CheckCollisionRecs(enemy->rect, Rectangle{
        .x = (float)BLOCK_SIZE * column, 
        .y = (float)BLOCK_SIZE * row, 
        .width = BLOCK_SIZE, 
        .height = BLOCK_SIZE
      })
    ){
      enemy->rect.x = (float)BLOCK_SIZE * (enemy->colPos);
      enemy->rect.y = (float)BLOCK_SIZE * (enemy->rowPos);

      return true;
    }
    break;
  }
  default:
    break;
  }
  // std::cout << "BOMB\n";
  {
    int adjacentRows[8]    = {-1, -1, -1, 0, 0, 1, 1, 1};  // row offsets
    int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};  // column offsets
    std::lock_guard<std::mutex> lock(bombsMutex);
    for (int i = 0; i < 8; ++i) {
      int tempRow = enemy->rowPos + adjacentRows[i];
      int tempColumn = enemy->colPos + adjacentColumns[i];

      if(WithinBoundary(tempRow, tempColumn))
      {
        for(auto& bomb : bombsArr){
          if(bomb != nullptr && 
          bomb->HasSamePosition(tempRow, tempColumn) &&
          !bomb->IsIdPresent(id)){
            if(CheckCollisionRecs(enemy->rect, 
                                  Rectangle{
                                    .x = (float)BLOCK_SIZE * tempColumn, 
                                    .y = (float)BLOCK_SIZE * tempRow, 
                                    .width = BLOCK_SIZE, 
                                    .height = BLOCK_SIZE
                                  }))
            { // TODO: Maybe change this collision checking with the bomb function and not here 
              enemy->rect.x = (float)BLOCK_SIZE * (enemy->colPos);
              enemy->rect.y = (float)BLOCK_SIZE * (enemy->rowPos);
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
  if(map[row][column].IsBlock()){
    return true;
  }
  return false;
}

bool Map::CollidingWithEnemy(int id)
{
  if(players.find(id) == players.end()) return false;

  std::shared_ptr<Player> player = players.at(id);

  if(!player->alive) return false;

  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(it->second->alive && CheckCollisionRecs(player->rect, it->second->rect)){
      return true;
    }
  }
  return false;
}

void Map::CollidingWithPowerUp(int id)
{
  if(players.find(id) == players.end()) return;

  std::shared_ptr<Player> player = players.at(id);

  if(!player->alive) return;

  int adjacentRows[8]    = {-1, -1, -1, 0, 0, 1, 1, 1};  // row offsets
  int adjacentColumns[8] = {-1, 0, 1, -1, 1, -1, 0, 1};  // column offsets

  for (int i = 0; i < 8; ++i) { 
    int newRow = player->rowPos + adjacentRows[i];
    int newCol = player->colPos + adjacentColumns[i];

    if(WithinBoundary(newRow, newCol)){
      if(map[newRow][newCol].powerUp != Power::NO_POWER && 
        map[newRow][newCol].CollidingWithPowerUp(player->rect)
      ){
        switch (map[newRow][newCol].powerUp)
        {
        case Power::SPEED:
        {
          // std::cout << "SPEED" << std::endl;
          player->speed *= 1.20;
          map[newRow][newCol].SetPowerType(Power::NO_POWER);
          break;
        }
        case Power::BOMB_RADIUS:
        {
          // std::cout << "RADIUS" << std::endl;
          player->bombRadius += 1;
          map[newRow][newCol].SetPowerType(Power::NO_POWER);
          break;
        }
        case Power::BOMB_COUNT:
        {
          // std::cout << "COUNT" << std::endl;
          player->bombCount += 1;
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
  for(auto position : explodeArr){
    if(Vector2Equal(map[position.first][position.second].texturePart, MIDDLE)) continue;

    int top     = CheckIfExplosion(position.first-1, position.second);
    int bottom  = CheckIfExplosion(position.first+1, position.second);
    int left    = CheckIfExplosion(position.first, position.second-1);
    int right   = CheckIfExplosion(position.first, position.second+1);

    int res = top + bottom + left + right;

    if(res == 0) continue;

    switch(res){
      case 0: continue; 
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
        map[position.first][position.second].texturePart = THREE_MERGE;
        if(top != 1){
          map[position.first][position.second].rotation = 0;
        }else if(bottom != 1){
          map[position.first][position.second].rotation = 180;
        }else if(left != 1){
          map[position.first][position.second].rotation = -90;
        }else if(right != 1){
          map[position.first][position.second].rotation = 90;
        }
        break;
      }
      case 4:{
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
  // return false;
  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    if(it->second->alive){
      return false;
    }
  }
  return true;
}

bool Map::IfPlayerDied(){
  if(!players.at(playerId)->alive){
    return true;
  }
  return false;
}

