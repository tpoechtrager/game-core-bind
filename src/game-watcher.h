#pragma once
#include <string>

namespace gamewatcher {

// Global flag set to true while a known game is running
extern bool GameRunning;

// Call this periodically to update GameRunning status
void Process();

// Resets internal state (clears GameRunning, active PID and game info)
void ResetState();

} // namespace gamewatcher
