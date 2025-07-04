#pragma once
#include <string>
#include <ctime>

namespace tools {

void RunDetached(const std::string& exe, const std::string& args);
std::time_t GetFileTimestamp(const std::string& path);
void SetWorkingDirToExePath();
void SleepMs(int ms);

} // namespace tools
