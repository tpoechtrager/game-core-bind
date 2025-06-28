#pragma once
#include <vector>

namespace scheduler {

// Binds given PID to specific OS thread IDs (zero-based)
bool BindProcessToThreads(int pid, const std::vector<int>& threads);

// Returns list of thread IDs the process is currently bound to
std::vector<int> GetProcessThreads(int pid);

} // namespace scheduler
