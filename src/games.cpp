#include "games.h"
#include <unordered_map>
#include <string>

namespace games {

static std::unordered_map<std::string, Game> GameMap;

void ClearList() {
  GameMap.clear();
}

void AddGame(const std::string& name, const std::string& binary) {
  GameMap[binary] = { name, binary };
}

const Game* GetGameByBinary(const std::string& binary) {
  auto it = GameMap.find(binary);
  if (it != GameMap.end()) {
    return &it->second;
  }
  return nullptr;
}

} // namespace games
