
#include "block.h"
#include "macros.h"
#include <iostream>

Block::Block(int row, int column, char type, Power powerUp) 
      : row(row), 
        column(column), 
        explodeTimer(0),
        type(type),
        powerUp(powerUp),
        rotation(0),
        powerUpRotate(0)
{

  int random = GetRandomValue(0,50);
  if(random < 40){
    grasstexture = getTexture(t_GRASS1);
  }else if(random < 45){
    grasstexture = getTexture(t_GRASS2);
  }else{
    grasstexture = getTexture(t_GRASS3);
  }
  
}

Block::Block(int row, int column, char type, Power powerUp, Model* model) 
      : row(row), 
        column(column), 
        explodeTimer(0),
        type(type),
        powerUp(powerUp),
        rotation(0),
        grassModel(model)
{
  int random = GetRandomValue(0,50);
  if(random < 40){
    grasstexture = getTexture(t_GRASS1);
  }else if(random < 45){
    grasstexture = getTexture(t_GRASS2);
  }else{
    grasstexture = getTexture(t_GRASS3);
  }
}

void Block::Draw()
{
  float scale = std::min(BLOCK_SIZE / grasstexture.width, BLOCK_SIZE / grasstexture.height);
  
  DrawTextureEx(grasstexture, Vector2{(float)(BLOCK_SIZE * column),(float)(BLOCK_SIZE * row)}, 0.0f, scale, WHITE);


  switch (powerUp)
  {
  case Power::SPEED:
  {
    // std::cout << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << " " << std::endl;
    Texture2D tex = getTexture(t_SPEED);  // Get the texture
    float scale = std::min(rect.width / tex.width, rect.height / tex.height);

    DrawTextureEx(tex, Vector2{rect.x,rect.y}, 0.0f, scale, WHITE);
    break;
  }
  case Power::BOMB_RADIUS:
  {
    Texture2D tex = getTexture(t_BOMB_RADIUS);  // Get the texture
    float scale = std::min(rect.width / tex.width, rect.height / tex.height);

    DrawTextureEx(tex, Vector2{rect.x,rect.y}, 0.0f, scale, WHITE);
    break;
  }
  case Power::BOMB_COUNT:
  {
    Texture2D tex = getTexture(t_BOMB_COUNT);  // Get the texture
    float scale = std::min(rect.width / tex.width, rect.height / tex.height);

    DrawTextureEx(tex, Vector2{rect.x,rect.y}, 0.0f, scale, WHITE);
    break;
  }
  default:
    break;
  }

  switch (type)
  {
  case BRICK_BLOCK:
  {
    Texture2D tex = getTexture(t_BRICK);  // Get the texture
    float scale = std::min(BLOCK_SIZE / tex.width, BLOCK_SIZE / tex.height);

    DrawTextureEx(tex, Vector2{(float)(BLOCK_SIZE * column),(float)(BLOCK_SIZE * row)}, 0.0f, scale, WHITE);
    break;
  }
  case CLAY_BLOCK:
  {
    Texture2D tex = getTexture(t_CLAY);  // Get the texture
    float scale = std::min(BLOCK_SIZE / tex.width, BLOCK_SIZE / tex.height);

    DrawTextureEx(tex, Vector2{(float)(BLOCK_SIZE * column),(float)(BLOCK_SIZE * row)}, 0.0f, scale, WHITE);
    break;
    break;
  }
  case EXPLODE:
  {
    Texture2D tex = getTexture(t_EXPLOSION);  // Get the texture

    DrawTexturePro(tex,
                    Rectangle{
                      texturePart.x, texturePart.y, 32, 32
                    }, 
                    Rectangle{
                      (float)(BLOCK_SIZE * column) + BLOCK_SIZE/2,
                      (float)(BLOCK_SIZE * row) + BLOCK_SIZE/2, 
                      (float)BLOCK_SIZE, 
                      (float)BLOCK_SIZE
                    },
                    (Vector2){ BLOCK_SIZE/2,BLOCK_SIZE/2 }, rotation, WHITE);
    
    break;
  }
  
  default:
    break;
  }
  
}

void Block::Draw3d()
{
  if(grassModel == nullptr) return;

  DrawModel(*grassModel, 
    Vector3{column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2 - 0.05f, 0.0f, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2 - 0.05f},
    1.1, WHITE
  );

  // TODO: New to draw powerup models as well
  if(powerUp != Power::NO_POWER){
    powerUpRotate += ROTATION_SPEED;
  }
  switch (powerUp)
  {
  case Power::SPEED:
  {
    std::shared_ptr<Model> powerUpModel = getModel(m_SPEED); 
    powerUpModel->transform = MatrixRotateX(DEG2RAD * -40);

    BoundingBox box = GetModelBoundingBox(*powerUpModel);
    
    DrawModelEx(*powerUpModel, 
    {
      column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, 
      (box.max.y-box.min.y)/2, 
      row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2
    }, 
    { 0.0f, 1.0f, 0.0f }, 
    powerUpRotate, 
    { .6f, .6f, .6f }, GOLD);
    break;
  }
  case Power::BOMB_RADIUS:
  {
    std::shared_ptr<Model> powerUpModel = getModel(m_BOMB_RADIUS); 
    powerUpModel->transform = MatrixRotateZ(DEG2RAD * 40);
    
    BoundingBox box = GetModelBoundingBox(*powerUpModel);
    
    DrawModelEx(*powerUpModel, 
    {
      column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, 
      (box.max.y-box.min.y)/2, 
      row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2
    }, 
    { 0.0f, 1.0f, 0.0f }, 
    powerUpRotate, 
    { .6f, .6f, .6f }, BLUE);
    break;
  }
  case Power::BOMB_COUNT:
  {
    std::shared_ptr<Model> powerUpModel = getModel(m_BOMB_COUNT); 

    powerUpModel->transform = MatrixRotateZ(DEG2RAD * 40);
    
    BoundingBox box = GetModelBoundingBox(*powerUpModel);
    
    DrawModelEx(*powerUpModel, 
    {
      column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, 
      (box.max.y-box.min.y)/2, 
      row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2
    }, 
    { 0.0f, 1.0f, 0.0f }, 
    powerUpRotate, 
    { .4f, .4f, .4f }, WHITE);
    break;
  }
  default:
    break;
  }

  switch (type)
  {
  case BRICK_BLOCK:
  {
    std::shared_ptr<Model> block = getModel(m_BRICK); 
    
    DrawModel(*block, 
      Vector3{column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, 0.0f, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2},
      1, WHITE
    );
    
    break;
  }
  case CLAY_BLOCK:
  {
    std::shared_ptr<Model> block = getModel(m_BOX);  
    
    DrawModel(*block, 
      Vector3{column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2, 0.0f, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2},
      1, WHITE
    );
    break;
  }
  case EXPLODE:
  {
    DrawModel(*grassModel, 
    Vector3{column * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2 - 0.05f, 0.0f, row * BLOCK_SIZE_3D + BLOCK_SIZE_3D/2 - 0.05f},
    1.1, RED
  );
    break;
  }
  
  default:
    break;
  }
}


bool Block::IsBlock() // Is the block a unpassable block like brick or clay
{
  return (IsBrick() || IsClay());
}

bool Block::IsBrick()
{
  return type == BRICK_BLOCK;
}

bool Block::IsClay()
{
  return type == CLAY_BLOCK;
}

void Block::SetType(char type)
{
  this->type = type;
}

void Block::SetExploadeTimer()
{
  explodeTimer = GetTime();
}

bool Block::ClearExplode()
{
  float currentTime = GetTime();
  
  if (currentTime - explodeTimer > EXPLODE_TIMER) { 
    explodeTimer = 0; 
    return true;
  }
  return false;
}

bool Block::CollidingWithPowerUp(Rectangle rect)
{
  if(CheckCollisionRecs(rect, this->rect)){
    return true;
  }
  return false;
}

void Block::SetPowerType(Power power)
{
  powerUp = power;
  switch (power)
    {
    case Power::SPEED:
    {
      rect = {
        (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * 0.2f)),
        (float)((BLOCK_SIZE * row)+ (BLOCK_SIZE * 0.2f)),
        (float)(BLOCK_SIZE  * .6),
        (float)(BLOCK_SIZE  * .6)
      };
      break;
    }
    case Power::BOMB_RADIUS:
    {
      rect = {
        (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * 0.1f)),
        (float)((BLOCK_SIZE * row)+ (BLOCK_SIZE * 0.1f)),
        (float)(BLOCK_SIZE  * .8),
        (float)(BLOCK_SIZE  * .8)
      };
      break;
    }
    case Power::BOMB_COUNT:
    {
      rect = {
        (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * 0.1f)),
        (float)((BLOCK_SIZE * row)+ (BLOCK_SIZE * 0.1f)),
        (float)(BLOCK_SIZE  * .8),
        (float)(BLOCK_SIZE  * .8)
      };
      break;
    }
    default:
      rect = {
        0,0,0,0
      };
      break;
    }
}

