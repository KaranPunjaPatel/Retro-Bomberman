#include "level.h"

std::vector<Level> allLevels{};

void loadLevels()
{
  allLevels.push_back(Level{}); // 0
  allLevels.push_back(Level{    // 1
    .type     = LvlType::OFFLINE,
    .rows     = 7,
    .columns  = 11,
    .map = {
      //0   1   2   3   4   5   6   7   8   9   10
      {'.','.','!','!','!','.','!','!','.','.','.'}, // 0
      {'.','#','!','#','!','#','!','#','.','#','!'}, // 1
      {'!','.','.','.','.','.','.','.','.','!','!'}, // 2
      {'!','#','!','#','.','#','!','#','.','#','.'}, // 3
      {'!','!','.','.','.','.','!','!','.','.','.'}, // 4
      {'.','#','!','#','.','#','.','#','.','#','!'}, // 5
      {'.','.','.','.','.','.','.','.','.','!','!'}, // 6
    },
    .powerUps = {
      { 0,  4, Power::SPEED       }, 
      { 2,  0, Power::BOMB_COUNT  }, 
      { 2, 10, Power::BOMB_RADIUS }
    },
    .enemies = {
      {.row = 0, .column = 9, .direction = Direction::WEST},
      {.row = 6, .column = 0, .direction = Direction::NORTH},
      {.row = 2, .column = 4, .direction = Direction::EAST}
    },
    .speed      = DEF_SPEED,
    .bombCount  = DEF_BOMBS,
    .bombRadius = DEF_BOMB_RADIUS,
  });
  allLevels.push_back(Level{    // 2
    .type     = LvlType::OFFLINE,
    .rows     = 11,
    .columns  = 17,
    .map = {
      //0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
      {'.','.','!','.','!','!','!','.','.','.','.','.','.','.','.','.','.'}, // 0
      {'.','#','.','#','.','#','!','#','.','#','.','#','.','#','.','#','!'}, // 1
      {'!','.','.','.','.','.','.','.','.','.','!','.','.','!','.','!','!'}, // 2
      {'.','#','.','#','.','#','!','#','.','#','.','#','!','#','.','#','.'}, // 3
      {'.','!','.','.','.','.','.','!','.','.','.','.','!','!','.','.','.'}, // 4
      {'.','#','.','#','.','#','.','#','.','#','!','#','.','#','!','#','.'}, // 5
      {'.','.','.','.','.','.','.','.','.','!','!','!','.','.','.','.','.'}, // 6
      {'!','#','.','#','.','#','!','#','.','#','!','#','.','#','.','#','.'}, // 7
      {'.','.','.','.','.','.','.','.','!','!','.','.','.','.','!','!','!'}, // 8
      {'.','#','.','#','.','#','.','#','!','#','.','#','.','#','!','#','.'}, // 9
      {'.','.','!','.','.','!','.','.','.','.','.','.','.','.','.','.','.'}, // 10
    },
    .powerUps = {
      { 0,  4, Power::SPEED       }, 
      { 2, 16, Power::SPEED       }, 
      { 2,  0, Power::BOMB_COUNT  }, 
      { 7,  0, Power::BOMB_COUNT  }, 
      { 8, 14, Power::BOMB_RADIUS }, 
      { 2, 13, Power::BOMB_RADIUS }
    },
    .enemies = {
      {.row =  0, .column = 16, .direction = Direction::WEST},
      {.row = 10, .column =  0, .direction = Direction::NORTH},
      {.row =  4, .column =  6, .direction = Direction::NORTH},
      {.row = 10, .column =  6, .direction = Direction::EAST},
      {.row =  4, .column = 14, .direction = Direction::NORTH}
    },
    .speed      = DEF_SPEED,
    .bombCount  = DEF_BOMBS,
    .bombRadius = DEF_BOMB_RADIUS,
  });
}

Level GetLevel(int index){
  if(index == 0){
    return GetRandomLevel();
  }else if(index == -1){
    return GetOneVsOneLevel();
  }else{
    return allLevels[index];
  }
}

Level GetRandomLevel(){
  Level level;
  level.type = LvlType::OFFLINE;

  level.rows = GetRandomValue(1,50);
  if(level.rows % 2 == 0){
    level.rows += 1;
  }
  level.columns = GetRandomValue(1,50);
  if(level.columns % 2 == 0){
    level.columns += 1;
  }

  level.map.resize(level.rows);
  for(size_t i = 0; i < level.rows; i++){
    level.map[i].resize(level.columns);
  }


  /* ============= Making the map ================= */
  for(size_t i = 0; i < level.rows; i++){
    for(size_t j = 0; j < level.columns; j++){
      if(i%2 == 0)
      {
        int random = GetRandomValue(1,10);
        if(random < 6){
          level.map[i][j] = '!';
        }else{
          level.map[i][j] = '.';
        }
      }
      else
      {
        if(j % 2 == 1){
          level.map[i][j] = '#';
        }else{
          int random = GetRandomValue(1,10);
          if(random < 6){
            level.map[i][j] = '!';
          }else{
            level.map[i][j] = '.';
          }
        }
      }
    }
  }

  level.map[0][0] = '.';
  level.map[0][1] = '.';
  level.map[1][0] = '.';
  level.map[0][2] = '!';
  level.map[2][0] = '!';

  /* ============= Assigning power Ups ================= */
  for(size_t i = 0; i < level.rows; i++){
    for(size_t j = 0; j < level.columns; j++){
      if(level.map[i][j] == '!'){
        int randomNumber = GetRandomValue(1,100);

        if (randomNumber > 85){            // 85% - 0
          if (randomNumber > 95){          //  5% - 1
            level.powerUps.push_back({i,j,Power::SPEED});
          } else if (randomNumber > 90){    //  5% - 2
            level.powerUps.push_back({i,j,Power::BOMB_RADIUS});
          } else if (randomNumber > 85){    //  5% - 3
            level.powerUps.push_back({i,j,Power::BOMB_COUNT});
          }
        }
      }
    }
  }

  /* ============= Putting enemies ================= */
  for(size_t i = 0; i < level.rows; i++){
    for(size_t j = 0; j < level.columns; j++){
      if((i == 0 && j == 0) || (i == 0 && j == 1) || (i == 1 && j == 0)) continue;
      if(level.map[i][j] == '.'){
        
        int randomNumber = GetRandomValue(1,4);

        if (randomNumber == 1){  
          level.enemies.push_back({i,j,0,Direction::NORTH});
        }
      }
    }
  }

  level.speed      = DEF_SPEED;
  level.bombCount  = DEF_BOMBS;
  level.bombRadius = DEF_BOMB_RADIUS;

  return level;
}

Level GetOneVsOneLevel(){
  Level level;

  level.type = LvlType::ONE_VS_ONE;

  level.rows = 11;
  level.columns = 17;

  level.map.resize(level.rows);
  for(size_t i = 0; i < level.rows; i++){
    level.map[i].resize(level.columns);
  }


  /* ============= Making the map ================= */
  for(size_t i = 0; i < level.rows; i++){
    for(size_t j = 0; j < level.columns; j++){
      if(i%2 == 0)
      {
        int random = GetRandomValue(1,10);
        if(random < 6){
          level.map[i][j] = '!';
        }else{
          level.map[i][j] = '.';
        }
      }
      else
      {
        if(j % 2 == 1){
          level.map[i][j] = '#';
        }else{
          int random = GetRandomValue(1,10);
          if(random < 6){
            level.map[i][j] = '!';
          }else{
            level.map[i][j] = '.';
          }
        }
      }
    }
  }

  level.map[0][0] = '.';
  level.map[0][1] = '.';
  level.map[1][0] = '.';
  level.map[0][2] = '!';
  level.map[2][0] = '!';

  level.map[level.rows-1][level.columns-1] = '.';
  level.map[level.rows-1][level.columns-2] = '.';
  level.map[level.rows-2][level.columns-1] = '.';
  level.map[level.rows-1][level.columns-3] = '!';
  level.map[level.rows-3][level.columns-1] = '!';

  /* ============= Assigning power Ups ================= */
  for(size_t i = 0; i < level.rows; i++){
    for(size_t j = 0; j < level.columns; j++){
      if(level.map[i][j] == '!'){
        int randomNumber = GetRandomValue(1,100);

        if (randomNumber > 85){            // 85% - 0
          if (randomNumber > 95){          //  5% - 1
            level.powerUps.push_back({i,j,Power::SPEED});
          } else if (randomNumber > 90){    //  5% - 2
            level.powerUps.push_back({i,j,Power::BOMB_RADIUS});
          } else if (randomNumber > 85){    //  5% - 3
            level.powerUps.push_back({i,j,Power::BOMB_COUNT});
          }
        }
      }
    }
  }

  /* ============= Putting players ================= */
  level.players.push_back({0,0,0,Direction::EAST});
  level.players.push_back({level.rows-1,level.columns-1,0,Direction::WEST});

  level.speed      = 3.5f;
  level.bombCount  = 1;
  level.bombRadius = 1;

  return level;
}