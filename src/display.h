#pragma once
#include <string>
#include <vector>

namespace display {

// Holds complete monitor configuration
struct MonitorInfo {
  std::string deviceName;
  std::string deviceString;
  std::string monitorName;
  bool isActive = false;
  bool isPrimary = false;

  int posX = 0;
  int posY = 0;
  int width = 0;
  int height = 0;
  int bitsPerPel = 32;
  int displayFrequency = 60;
};

// Returns all monitors with full information
std::vector<MonitorInfo> GetMonitors();

// Disables a monitor by device name
bool DisableMonitor(const std::string& deviceName);

// Enables a monitor by device name using stored settings
bool EnableMonitor(const MonitorInfo& info);

} // namespace display
