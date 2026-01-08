
#include "player.h"
#include <iostream>
#include <cmath>
#include "map.h"
// #include "util.h"

Player::Player(std::shared_ptr<Map> map, int id, size_t row, size_t column) : Human(map, id, row, column)
{
  lifeCount = 2;

  direction = Direction::CENTER;

  texture = getTexture(t_PLAYER);  // Get the texture

  float ratio = (float)(texture.width)/texture.height;
  // std::cout << tex.width << " " << tex.height << " " << ratio << std::endl;

  if(SHOW_2D == 1){
    rect = {
      .x = (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * .2f)/2), 
      .y = (float)((BLOCK_SIZE * row) + (BLOCK_SIZE * .2f)/2), 
      .width  = BLOCK_SIZE * .8f * ratio, //49.6, 
      .height = BLOCK_SIZE * .8f  //60 
    };
  }else{
    model = getModel(m_PLAYER);
    animation = getAnimation(m_PLAYER).first;
    animCount = getAnimation(m_PLAYER).second;

    rect = {
      .x = (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * .3f)/2),
      .y = (float)((BLOCK_SIZE * row) + (BLOCK_SIZE * .3f)/2),
      .width  = BLOCK_SIZE * .7f, 
      .height = BLOCK_SIZE * .7f  
    };
  }
  

}

Player::~Player()
{

}

void Player::Draw()
{
  float scale = std::min(rect.width / texture.width, rect.height / texture.height);

  DrawTextureEx(texture, Vector2{rect.x,rect.y}, 0.0f, scale, WHITE);
}

void Player::Draw3d()
{
  // DrawModel(*model,
  //   {rect.x/BLOCK_SIZE + BLOCK_SIZE_3D/2, 0.0f, rect.y/BLOCK_SIZE + BLOCK_SIZE_3D/2}, 
  //   .6f, WHITE);
  // DrawModelEx(model,{ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 40.0f, { 10.0f, 10.0f, 10.0f }, WHITE);
  DrawModelEx(*model, 
    {rect.x/BLOCK_SIZE + BLOCK_SIZE_3D/2, 0.0f, rect.y/BLOCK_SIZE + BLOCK_SIZE_3D/2}, 
    { 0.0f, 1.0f, 0.0f }, 
    (float)rotationAngle, 
    { .6f, .6f, .6f }, WHITE);
  // DrawModelEx(model,{ rect.x, rect.y, 0.0f }, { 0.0f, 1.0f, 0.0f }, rotationAngle, { 10.0f, 10.0f, 10.0f }, WHITE);
}


void Player::HandleInput()
{
  if(!alive) return;
  
  // std::cout << speed << " " << (speed * GetFrameTime()) << "\n";
  
  if(IsKeyDown(KEY_A)){
    rect.x -= (speed * GetFrameTime());
    if(std::abs(rotationAngle - (-90)) < rotateSpeed){
      rotationAngle = -90;
    }else if(rotationAngle > -90 && rotationAngle < 90){
      rotationAngle -= rotateSpeed;
    }else{
      rotationAngle += rotateSpeed;
    }

    if(map->IsColliding(id))
    {
      rect.x += (speed * GetFrameTime());
    }
    if(rect.x < 0) rect.x = 0;
    SetDirection(Direction::WEST);
  }

  if(IsKeyDown(KEY_D)){
    rect.x += (speed * GetFrameTime());
    if(std::abs(rotationAngle - (90)) < rotateSpeed){
      rotationAngle = 90;
    }else if(rotationAngle < 90 && rotationAngle > -90){
      rotationAngle += rotateSpeed;
    }else{
      rotationAngle -= rotateSpeed;
    }
    if(map->IsColliding(id))
    {
      rect.x -= (speed * GetFrameTime());
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen width with the bounds of the level inside of the window
    if(rect.x + rect.width > map->mapWidth) rect.x = map->mapWidth - rect.width; 
    SetDirection(Direction::EAST);
  }

  if(IsKeyDown(KEY_W)){
    rect.y -= (speed * GetFrameTime());
    if(std::abs(rotationAngle - (180)) < rotateSpeed){
      rotationAngle = 180;
    }else if(rotationAngle < 0){
      rotationAngle -= rotateSpeed;
    }else{
      rotationAngle += rotateSpeed;
    }
    if(map->IsColliding(id))
    {
      rect.y += (speed * GetFrameTime());
    }
    if(rect.y < 0) rect.y = 0;
    SetDirection(Direction::NORTH);

    
  }
  if(IsKeyDown(KEY_S)){
    rect.y += (speed * GetFrameTime());
    if(std::abs(rotationAngle - (0)) < rotateSpeed){
      rotationAngle = 0;
    }else if(rotationAngle > 0){
      rotationAngle -= rotateSpeed;
    }else{
      rotationAngle += rotateSpeed;
    }
    if(map->IsColliding(id))
    {
      rect.y -= (speed * GetFrameTime());
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen height with the bounds of the level inside of the window
    if(rect.y + rect.height > map->mapHeight) rect.y = map->mapHeight - rect.height;
    SetDirection(Direction::SOUTH);
  }

  rotationAngle = normalizeAngle(rotationAngle);

  if(map->IsInExplosion(id)){
    // TODO: Work on the die function
    Dies();
  }

  map->ChangeBombIds(id);
  
  ChangePosition();

  if(map->CollidingWithEnemy(id)){
    Dies();
  }

  map->CollidingWithPowerUp(id);

  if(alive && IsKeyDown(KEY_SPACE))
  {
    if(bombCount > 0 && !map->AlreadyHasBomb(rowPos, colPos)){
      map->AddBomb(id);
      bombCount--;
    }
  }

  if(alive && !IsKeyDown(KEY_A) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_D) && !IsKeyDown(KEY_S)){
    SetDirection(Direction::CENTER);
    UpdateModelAnimation(*model, animation[36], animFrame);
    if (animFrame >= animation[36].frameCount) {
      animFrame = 0;
    }
    animFrame++;
  }else{
    UpdateModelAnimation(*model, animation[49], animFrame);
    if (animFrame >= animation[49].frameCount) {
      animFrame = 0;
    }
    animFrame++;
  }

}

void Player::HandleInput2()
{
  if(!alive) return;
  
  if(IsKeyDown(KEY_LEFT)){
    rect.x -= speed;
    if(map->IsColliding(id))
    {
      rect.x += speed;
    }
    if(rect.x < 0) rect.x = 0;
    SetDirection(Direction::WEST);
  }
  if(IsKeyDown(KEY_RIGHT)){
    rect.x += speed;
    if(map->IsColliding(id))
    {
      rect.x -= speed;
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen width with the bounds of the level inside of the window
    if(rect.x + rect.width > map->mapWidth) rect.x = map->mapWidth - rect.width; 
    SetDirection(Direction::EAST);
  }
  if(IsKeyDown(KEY_UP)){
    rect.y -= speed;
    if(map->IsColliding(id))
    {
      rect.y += speed;
    }
    if(rect.y < 0) rect.y = 0;
    SetDirection(Direction::NORTH);
  }
  if(IsKeyDown(KEY_DOWN)){
    rect.y += speed;
    if(map->IsColliding(id))
    {
      rect.y -= speed;
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen height with the bounds of the level inside of the window
    if(rect.y + rect.height > map->mapHeight) rect.y = map->mapHeight - rect.height;
    SetDirection(Direction::SOUTH);
  }

  if(map->IsInExplosion(id)){
    // TODO: Work on the die function
    Dies();
  }

  map->ChangeBombIds(id);
  
  ChangePosition();

  if(map->CollidingWithEnemy(id)){
    Dies();
  }

  map->CollidingWithPowerUp(id);

  if(alive && IsKeyDown(KEY_KP_0))
  {
    if(bombCount > 0 && !map->AlreadyHasBomb(rowPos, colPos)){
      map->AddBomb(id);
      bombCount--;
    }
  }

  if(alive && !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_UP) && !IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_DOWN)){
    SetDirection(Direction::CENTER);
  }
}

void Player::Dies()
{
  lifeCount--;

  if(lifeCount <= 0){
    alive = false;
    std::cout << "Player Died" << std::endl;
  }

  float ratio = (float)(texture.width)/texture.height;
  rect = {
    .x = (float)((BLOCK_SIZE * 0) + (BLOCK_SIZE * .2f)/2), 
    .y = (float)((BLOCK_SIZE * 0) + (BLOCK_SIZE * .2f)/2), 
    .width  = BLOCK_SIZE * .8f * ratio, //49.6, 
    .height = BLOCK_SIZE * .8f  //60 
  };
  rowPos = 0;
  colPos = 0;

  ChangePosition();
}

