#include "scheduler.h"
#include <vector>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <sched.h>
#include <unistd.h>
#endif

namespace scheduler {

bool BindProcessToThreads(int pid, const std::vector<int>& threads) {
  if (threads.empty()) {
    return false;
  }

#ifdef _WIN32
  DWORD_PTR mask = 0;
  for (int t : threads) {
    if (t < 0 || t >= (int)(sizeof(DWORD_PTR) * 8)) {
      printf("Invalid thread index: %d\n", t);
      return false;
    }
    mask |= (1ULL << t);
  }

  HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
  if (!hProcess) {
    printf("Failed to open process: %d\n", pid);
    return false;
  }

  BOOL result = SetProcessAffinityMask(hProcess, mask);
  CloseHandle(hProcess);
  return result != 0;

#else
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);
  
  for (int t : threads) {
    if (t < 0 || t >= CPU_SETSIZE) {
      printf("Invalid thread index: %d\n", t);
      return false;
    }
    CPU_SET(t, &cpuSet);
  }

  int result = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuSet);
  return result == 0;
#endif
}

std::vector<int> GetProcessThreads(int pid) {
  std::vector<int> threads;

#ifdef _WIN32
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
  if (!hProcess) {
    printf("Failed to open process: %d\n", pid);
    return threads;
  }

  DWORD_PTR mask = 0, systemMask = 0;
  if (GetProcessAffinityMask(hProcess, &mask, &systemMask)) {
    for (int i = 0; i < (int)(sizeof(DWORD_PTR) * 8); ++i) {
      if (mask & (1ULL << i)) {
        threads.push_back(i);
      }
    }
  }

  CloseHandle(hProcess);

#else
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);

  if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpuSet) == 0) {
    for (int i = 0; i < CPU_SETSIZE; ++i) {
      if (CPU_ISSET(i, &cpuSet)) {
        threads.push_back(i);
      }
    }
  }
#endif

  return threads;
}

} // namespace scheduler
