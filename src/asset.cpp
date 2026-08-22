
#include "asset.h"

// Define the global vector
std::vector<Texture2D> textures{};
std::vector<Texture2D> textures3d{};

std::vector<std::shared_ptr<Model>> models{};
std::vector<std::pair<ModelAnimation *, int>> animations{};

std::atomic_bool b_textures{false};
std::atomic_bool b_models{false};
std::atomic_bool b_animations{false};

std::mutex assetMutex;

// Loads all the textures png needed
void loadTextures()
{
  textures.push_back(LoadTexture("./resources/grass1.png"));
  textures.push_back(LoadTexture("./resources/grass2.png"));
  textures.push_back(LoadTexture("./resources/grass3.png"));
  textures.push_back(LoadTexture("./resources/aititle.png"));
  textures.push_back(LoadTexture("./resources/brick.png"));
  textures.push_back(LoadTexture("./resources/clay.png"));
  textures.push_back(LoadTexture("./resources/player2.png"));
  textures.push_back(LoadTexture("./resources/goomba.png"));
  textures.push_back(LoadTexture("./resources/bomb.png"));
  textures.push_back(LoadTexture("./resources/explosion-all.png"));
  textures.push_back(LoadTexture("./resources/diamond.png"));
  textures.push_back(LoadTexture("./resources/emarald.png"));
  textures.push_back(LoadTexture("./resources/gold.png"));
  textures.push_back(LoadTexture("./resources/newbg.png"));
  // textures.push_back(LoadTexture("./resources/background.jpg"));
  textures.push_back(LoadTexture("./resources/heart.png"));
  textures.push_back(LoadTexture("./resources/bomb-radius.png"));
  textures.push_back(LoadTexture("./resources/bomb-count.png"));
  textures.push_back(LoadTexture("./resources/clock.png"));
}

void pushModel(const char *file, float width = 0.0f, float length = 0.0f)
{
  Model tempModel = LoadModel(file);

  BoundingBox box = GetModelBoundingBox(tempModel);

  std::cout << box.max.x << " " << box.min.x << "\n";
  std::cout << box.max.z << " " << box.min.z << "\n";
  std::cout << (box.max.x - box.min.x) << " " << (box.max.z - box.min.z) << "\n";

  if (width != 0.0f || length != 0.0f)
  {
    Matrix scaleMatrix;
    if (width != 0.0f && length != 0.0f)
    {
      scaleMatrix = MatrixScale(width / (box.max.x - box.min.x), 1.0f, length / (box.max.z - box.min.z));
    }
    else if (width != 0.0f)
    {
      scaleMatrix = MatrixScale(width / (box.max.x - box.min.x), 1.0f, 1.0f);
    }
    else if (length != 0.0f)
    {
      scaleMatrix = MatrixScale(1.0f, 1.0f, length / (box.max.z - box.min.z));
    }

    tempModel.transform = MatrixMultiply(tempModel.transform, scaleMatrix);
  }
  box = GetModelBoundingBox(tempModel);

  // std::cout << box.max.x << " " << box.min.x << "\n";
  // std::cout << box.max.z << " " << box.min.z << "\n";
  // std::cout << (box.max.x - box.min.x) << " " << (box.max.z - box.min.z) << "\n";

  auto modelPtr = std::shared_ptr<Model>(
      new Model(tempModel),
      [](Model *m)
      {
        UnloadModel(*m); // Properly unload raylib resources
        delete m;        // Free the allocated memory
      });

  // Now push the shared_ptr into the vector
  models.push_back(modelPtr);
}

// Loads all the models needed
void loadModels()
{
  pushModel("./resources/player.glb");
  pushModel("./resources/enemy.glb");
  pushModel("./resources/box.glb", BLOCK_SIZE_3D, BLOCK_SIZE_3D);
  pushModel("./resources/block-grass-large.glb", BLOCK_SIZE_3D, BLOCK_SIZE_3D);
  pushModel("./resources/block-grass-narrow.glb", BLOCK_SIZE_3D, BLOCK_SIZE_3D);
  pushModel("./resources/bomb2.glb");
  pushModel("./resources/bolt.glb");
  pushModel("./resources/star.glb");
  pushModel("./resources/bombcount.glb");
}

// Loads all the animation for the models needed
void loadAnimations()
{
  int animCount{};
  ModelAnimation *playerAnims = LoadModelAnimations("./resources/player.glb", &animCount);
  {
    std::lock_guard<std::mutex> lock(assetMutex);
    animations.push_back(std::make_pair(playerAnims, animCount));
  }

  ModelAnimation *enemyAnims = LoadModelAnimations("./resources/enemy.glb", &animCount);
  {
    std::lock_guard<std::mutex> lock(assetMutex);
    animations.push_back(std::make_pair(enemyAnims, animCount));
  }
}

Texture2D getTexture(int asset)
{
  while (!b_textures.load())
    std::this_thread::yield();

  std::lock_guard<std::mutex> lock(assetMutex);
  return textures[asset];
}

std::shared_ptr<Model> getModel(int asset)
{
  while (!b_models.load())
    std::this_thread::yield();

  std::lock_guard<std::mutex> lock(assetMutex);
  return models[asset];
}

std::pair<ModelAnimation *, int> getAnimation(int asset)
{
  while (!b_animations.load())
    std::this_thread::yield();

  std::lock_guard<std::mutex> lock(assetMutex);
  return animations[asset];
}

void unloadAssets()
{
  for (auto &texture : textures)
  {
    UnloadTexture(texture);
  }

  // for(auto &model : models)
  // {
  //   UnloadModel(*model);
  // }

  for (auto &pair : animations)
  {
    UnloadModelAnimations(pair.first, pair.second);
  }
}

void loadAssets()
{
  loadTextures();
  b_textures.store(true);

  loadModels();
  b_models.store(true);

  loadAnimations();
  b_animations.store(true);
}
