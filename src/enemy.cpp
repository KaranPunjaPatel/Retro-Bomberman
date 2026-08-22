
#include "enemy.h"
#include "map.h"

Enemy::Enemy(std::shared_ptr<Map> map, int id, size_t row, size_t column)
    : Human(map, id, row, column)
{
  int random = GetRandomValue(0, 3);

  direction = static_cast<Direction>(random);

  switch (direction)
  {
  case Direction::NORTH:
    rotationAngle = -180;
    break;
  case Direction::SOUTH:
    rotationAngle = 0;
    break;
  case Direction::EAST:
    rotationAngle = 90;
    break;
  case Direction::WEST:
    rotationAngle = -90;
    break;

  default:
    break;
  }

  texture = getTexture(t_ENEMY);

  rect = {
      .x = (float)(BLOCK_SIZE * column),
      .y = (float)(BLOCK_SIZE * row),
      .width = BLOCK_SIZE,
      .height = BLOCK_SIZE};

  model = getModel(m_ENEMY);
  animation = getAnimation(m_ENEMY).first;
  animCount = getAnimation(m_ENEMY).second;
}

Enemy::~Enemy() {}

void Enemy::Draw()
{
  // DrawRectangleRec(rect, BLACK);

  float scale =
      std::min(rect.width / texture.width, rect.height / texture.height);

  DrawTextureEx(texture, Vector2{rect.x, rect.y}, 0.0f, scale, WHITE);
}

void Enemy::Draw3d()
{
  // std::cout << "      [ENEMY] DrawModelEx starting for Enemy ID: " << id << std::endl;
  DrawModelEx(*model,
              {rect.x / BLOCK_SIZE + BLOCK_SIZE_3D / 2, 0.0f,
               rect.y / BLOCK_SIZE + BLOCK_SIZE_3D / 2},
              {0.0f, 1.0f, 0.0f}, rotationAngle, {.6f, .6f, .6f}, WHITE);
  // std::cout << "      [ENEMY] DrawModelEx complete for Enemy ID: " << id << std::endl;
}

void Enemy::Move()
{
  if (!alive)
    return;
  switch (direction)
  {
  case Direction::NORTH:
  {
    // std::cout<<"NORTH ";
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
    if (map->IsEnemyColliding(id))
    {
      // rect.y += speed;
      ChangeDirection();
      // Move();
    }

    break;
  }
  case Direction::EAST:
  {
    // std::cout<<"EAST ";
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

    if (map->IsEnemyColliding(id))
    {
      ChangeDirection();
    }
    break;
  }
  case Direction::SOUTH:
  {
    // std::cout<<"SOUTH ";
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

    if (map->IsEnemyColliding(id))
    {
      // rect.y -= speed;
      ChangeDirection();
      // Move();
    }
    break;
  }
  case Direction::WEST:
  {
    // std::cout<<"WEST ";
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

    if (map->IsEnemyColliding(id))
    {
      // rect.x += speed;
      ChangeDirection();
      // Move();
    }

    break;
  }
  default:
    break;
  }

  rotationAngle = normalizeAngle(rotationAngle);

  if (map->IsInExplosion(id))
  {
    // TODO: Work on the die function
    Dies();
  }
  if (alive)
  {
    map->ChangeBombIds(id);
    ChangePosition();
  }

  if (model != nullptr && animation != nullptr && animCount > 55)
  {
    ModelAnimation anim = animation[55];
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
}

void Enemy::ChangeDirection()
{
  int random = GetRandomValue(1, 3);

  SetDirection(
      static_cast<Direction>(((uint8_t)(this->direction) + random) % 4));
}

void Enemy::Dies()
{
  alive = false;
  std::cout << "Enemy Died" << std::endl;
}
