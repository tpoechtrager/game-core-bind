#pragma once

#include <string>

namespace lua {

// Initialize Lua state
void Init();

// Execute Lua code from string
void Execute(const char* code);

// Execute Lua script file
void ExecuteFile(const char* filename);

// Shutdown Lua state and cleanup
void Shutdown();

// Initialize onTick binding if present
void InitTick();

// Initialize foreground/background callbacks if present
void InitForegroundCallbacks();

// Trigger registered onTick function
void TriggerTick();

// Trigger onGameStart event
void TriggerGameStart(int pid, const std::string& name, const std::string& binary);

// Trigger onGameStop event
void TriggerGameStop(int pid, const std::string& name, const std::string& binary);

// Trigger onGameForeground event
void TriggerGameForeground(int pid, const std::string& name, const std::string& binary);

// Trigger onGameBackground event
void TriggerGameBackground(int pid, const std::string& name, const std::string& binary);

} // namespace lua
