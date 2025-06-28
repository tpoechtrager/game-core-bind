#pragma once
#include <string>
#include <ctime>

namespace tools {

std::time_t GetFileTimestamp(const std::string& path);
void SetWorkingDirToExePath();
void SleepMs(int ms);

} // namespace tools
