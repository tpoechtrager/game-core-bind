#include "games.h"
#include <unordered_map>
#include <string>
#include <algorithm>

namespace games {

static std::unordered_map<std::string, Game> GameMap;
static std::unordered_map<std::string, std::string> LowercaseBinaryMap;


void ClearList() {
  GameMap.clear();
  LowercaseBinaryMap.clear();
}

void AddGame(const std::string& name, const std::string& binary) {
  GameMap[name] = Game{ name, binary };
  std::string binaryLower = binary;
  std::transform(binaryLower.begin(), binaryLower.end(), binaryLower.begin(), ::tolower);
  LowercaseBinaryMap[binaryLower] = name;
}

const Game* GetGameByBinary(const std::string& binary, bool caseInsensitive) {
  if (!caseInsensitive) {
    for (const auto& [name, game] : GameMap) {
      if (game.binary == binary) {
        return &game;
      }
    }
    return nullptr;
  }

  std::string binaryLower = binary;
  std::transform(binaryLower.begin(), binaryLower.end(), binaryLower.begin(), ::tolower);
  auto it = LowercaseBinaryMap.find(binaryLower);
  if (it != LowercaseBinaryMap.end()) {
    auto gameIt = GameMap.find(it->second);
    if (gameIt != GameMap.end()) {
      return &gameIt->second;
    }
  }

  return nullptr;
}

} // namespace games
