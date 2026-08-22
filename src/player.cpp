
#include <iostream>
#include <cmath>

#include "player.h"
#include "map.h"
// #include "util.h"

const std::vector<KeyboardKey> CONTROLS_P1 = {KEY_A, KEY_D, KEY_W, KEY_S, KEY_SPACE};
const std::vector<KeyboardKey> CONTROLS_P2 = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_CONTROL};

Player::Player(std::shared_ptr<Map> map, int id, size_t row, size_t column, int player)
    : Human(map, id, row, column),
      player(player),
      lifeCount(2),
      startRow(row),
      startCol(column)
{
  direction = Direction::CENTER;

  texture = getTexture(t_PLAYER); // Get the texture

  if (SHOW_2D == 1)
  {
    float ratio = (float)(texture.width) / texture.height;
    rect = {
        .x = (float)((BLOCK_SIZE * column) + (BLOCK_SIZE * .2f) / 2),
        .y = (float)((BLOCK_SIZE * row) + (BLOCK_SIZE * .2f) / 2),
        .width = BLOCK_SIZE * .8f * ratio, // 49.6,
        .height = BLOCK_SIZE * .8f         // 60
    };
  }
  else
  {
    model = std::make_shared<Model>(LoadModel("./resources/player.glb"));
    animation = getAnimation(m_PLAYER).first;
    animCount = getAnimation(m_PLAYER).second;

    rect = {
        .x = (float)(BLOCK_SIZE * column) + BLOCK_SIZE * 0.2f,
        .y = (float)(BLOCK_SIZE * row) + BLOCK_SIZE * 0.2f,
        .width = BLOCK_SIZE * 0.6f,
        .height = BLOCK_SIZE * 0.6f};
  }
}

Player::~Player()
{
  if (model != nullptr)
  {
    UnloadModel(*model);
  }
}

void Player::Draw()
{
  float scale = std::min(rect.width / texture.width, rect.height / texture.height);

  DrawTextureEx(texture, Vector2{rect.x, rect.y}, 0.0f, scale, WHITE);
}

void Player::Draw3d()
{
  // DrawModel(*model,
  //   {rect.x/BLOCK_SIZE + BLOCK_SIZE_3D/2, 0.0f, rect.y/BLOCK_SIZE + BLOCK_SIZE_3D/2},
  //   .6f, WHITE);
  // DrawModelEx(model,{ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 40.0f, { 10.0f, 10.0f, 10.0f }, WHITE);
  // std::cout << "      [PLAYER] Draw3d starting..." << std::endl;
  DrawModelEx(*model,
              {(rect.x + rect.width / 2) / BLOCK_SIZE, 0.0f, (rect.y + rect.height / 2) / BLOCK_SIZE},
              {0.0f, 1.0f, 0.0f},
              (float)rotationAngle,
              {.6f, .6f, .6f}, WHITE);
  // DrawModelEx(model,{ rect.x, rect.y, 0.0f }, { 0.0f, 1.0f, 0.0f }, rotationAngle, { 10.0f, 10.0f, 10.0f }, WHITE);
  // std::cout << "      [PLAYER] Draw3d complete!" << std::endl;
}

void Player::HandleInput()
{
  if (!alive)
    return;
  const auto &keys = (player == PLAYER_ONE) ? CONTROLS_P1 : CONTROLS_P2;

  KeyboardKey keyLeft = keys[0];
  KeyboardKey keyRight = keys[1];
  KeyboardKey keyUp = keys[2];
  KeyboardKey keyDown = keys[3];
  KeyboardKey keyBomb = keys[4];

  // std::cout << "    [PLAYER_INPUT] 1. Key Controls Check..." << std::endl;

  if (IsKeyDown(keyLeft))
  {
    rect.x -= (speed * GetFrameTime());
    if (std::abs(rotationAngle - (-90)) < rotateSpeed)
    {
      rotationAngle = -90;
    }
    else if (rotationAngle > -90 && rotationAngle < 90)
    {
      rotationAngle -= rotateSpeed;
    }
    else
    {
      rotationAngle += rotateSpeed;
    }

    if (map->IsColliding(id))
    {
      rect.x += (speed * GetFrameTime());
    }
    if (rect.x < 0)
      rect.x = 0;
    SetDirection(Direction::WEST);
  }

  if (IsKeyDown(keyRight))
  {
    rect.x += (speed * GetFrameTime());
    if (std::abs(rotationAngle - (90)) < rotateSpeed)
    {
      rotationAngle = 90;
    }
    else if (rotationAngle < 90 && rotationAngle > -90)
    {
      rotationAngle += rotateSpeed;
    }
    else
    {
      rotationAngle -= rotateSpeed;
    }
    if (map->IsColliding(id))
    {
      rect.x -= (speed * GetFrameTime());
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen width with the bounds of the level inside of the window
    if (rect.x + rect.width > map->mapWidth)
      rect.x = map->mapWidth - rect.width;
    SetDirection(Direction::EAST);
  }

  if (IsKeyDown(keyUp))
  {
    rect.y -= (speed * GetFrameTime());
    if (std::abs(rotationAngle - (180)) < rotateSpeed)
    {
      rotationAngle = 180;
    }
    else if (rotationAngle < 0)
    {
      rotationAngle -= rotateSpeed;
    }
    else
    {
      rotationAngle += rotateSpeed;
    }
    if (map->IsColliding(id))
    {
      rect.y += (speed * GetFrameTime());
    }
    if (rect.y < 0)
      rect.y = 0;
    SetDirection(Direction::NORTH);
  }

  if (IsKeyDown(keyDown))
  {
    rect.y += (speed * GetFrameTime());
    if (std::abs(rotationAngle - (0)) < rotateSpeed)
    {
      rotationAngle = 0;
    }
    else if (rotationAngle > 0)
    {
      rotationAngle -= rotateSpeed;
    }
    else
    {
      rotationAngle += rotateSpeed;
    }
    if (map->IsColliding(id))
    {
      rect.y -= (speed * GetFrameTime());
    }
    // TODO: When the map is big and zooming is allowed it needs to change this get screen height with the bounds of the level inside of the window
    if (rect.y + rect.height > map->mapHeight)
      rect.y = map->mapHeight - rect.height;
    SetDirection(Direction::SOUTH);
  }

  rotationAngle = normalizeAngle(rotationAngle);

  // std::cout << "    [PLAYER_INPUT] 2. IsInExplosion Check..." << std::endl;
  if (map->IsInExplosion(id))
  {
    // TODO: Work on the die function
    Dies();
  }

  // std::cout << "    [PLAYER_INPUT] 3. ChangeBombIds..." << std::endl;
  map->ChangeBombIds(id);

  // std::cout << "    [PLAYER_INPUT] 4. ChangePosition..." << std::endl;
  ChangePosition();

  // std::cout << "    [PLAYER_INPUT] 5. CollidingWithEnemy..." << std::endl;
  if (map->CollidingWithEnemy(id))
  {
    Dies();
  }

  // std::cout << "    [PLAYER_INPUT] 6. CollidingWithPowerUp..." << std::endl;
  map->CollidingWithPowerUp(id);

  // std::cout << "    [PLAYER_INPUT] 7. Bomb Drop Check..." << std::endl;
  if (alive && IsKeyPressed(keyBomb))
  {
    if (bombCount > 0 && !map->AlreadyHasBomb(rowPos, colPos))
    {
      map->AddBomb(id);
      bombCount--;
    }
  }

  // std::cout << "    [PLAYER_INPUT] 8. Model Animation Update..." << std::endl;
  if (model != nullptr && animation != nullptr && animCount > 49)
  {
    bool isMoving = IsKeyDown(keyLeft) || IsKeyDown(keyRight) ||
                    IsKeyDown(keyUp) || IsKeyDown(keyDown);
    int animIdx = (alive && !isMoving) ? 36 : 49;
    ModelAnimation anim = animation[animIdx];
    if (animIdx == 36)
    {
      SetDirection(Direction::CENTER);
    }
    if (model->boneCount > 0 && anim.boneCount == model->boneCount && anim.frameCount > 0 && anim.framePoses != nullptr)
    {
      UpdateModelAnimation(*model, anim, animFrame);
      if (animFrame >= anim.frameCount)
      {
        animFrame = 0;
      }
      animFrame++;
    }
  }

  // std::cout << "    [PLAYER_INPUT] 9. Player HandleInput Complete!" << std::endl;
}

void Player::Dies()
{
  lifeCount--;

  if (lifeCount <= 0)
    alive = false;

  rowPos = startRow;
  colPos = startCol;

  if (SHOW_2D == 1)
  {
    float ratio = (float)(texture.width) / texture.height;
    rect = {
        .x = (float)((BLOCK_SIZE * startCol) + (BLOCK_SIZE * .2f) / 2),
        .y = (float)((BLOCK_SIZE * startRow) + (BLOCK_SIZE * .2f) / 2),
        .width = BLOCK_SIZE * .8f * ratio,
        .height = BLOCK_SIZE * .8f};
  }
  else
  {
    rect = {
        .x = (float)(BLOCK_SIZE * startCol) + BLOCK_SIZE * 0.2f,
        .y = (float)(BLOCK_SIZE * startRow) + BLOCK_SIZE * 0.2f,
        .width = BLOCK_SIZE * 0.6f,
        .height = BLOCK_SIZE * 0.6f};
  }

  ChangePosition();
}
