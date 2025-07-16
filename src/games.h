#pragma once
#include <string>

namespace games {

struct Game {
  std::string name;
  std::string binary;
};

// Removes all games
void ClearList();

// Adds a game with given name and binary
void AddGame(const std::string& name, const std::string& binary);

// Returns pointer to Game if binary matches, otherwise nullptr
const Game* GetGameByBinary(const std::string& binary, bool caseInsensitive = false);

} // namespace games
