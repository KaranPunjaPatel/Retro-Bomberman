#include "structure.h"


// '^' is for player
// '.' is empty spot
// '#' is a unbreakable block
// '!' is a breakable block
extern std::vector<Level> allLevels;
void loadLevels();

Level GetLevel(int index);

Level GetRandomLevel();
Level GetOneVsOneLevel();
