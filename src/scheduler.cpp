#include "scheduler.h"
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#endif

namespace scheduler {

BindResult BindProcessToThreads(int pid, const std::vector<int>& threads) {
  if (threads.empty()) {
    return BIND_INVALID_THREAD_INDEX;
  }

#ifdef _WIN32
  DWORD_PTR mask = 0;
  for (int t : threads) {
    if (t < 0 || t >= (int)(sizeof(DWORD_PTR) * 8)) {
      return BIND_INVALID_THREAD_INDEX;
    }
    mask |= (1ULL << t);
  }

  HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
  if (!hProcess) {
    return BIND_OPEN_PROCESS_FAILED;
  }

  BOOL result = SetProcessAffinityMask(hProcess, mask);
  CloseHandle(hProcess);

  if (!result) {
    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED) {
      return BIND_PERMISSION_DENIED;
    }
    return BIND_SETAFFINITY_FAILED;
  }

  return BIND_SUCCESS;

#else
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);

  for (int t : threads) {
    if (t < 0 || t >= CPU_SETSIZE) {
      return BIND_INVALID_THREAD_INDEX;
    }
    CPU_SET(t, &cpuSet);
  }

  if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuSet) != 0) {
    if (errno == EPERM) {
      return BIND_PERMISSION_DENIED;
    }
    return BIND_SETAFFINITY_FAILED;
  }

  return BIND_SUCCESS;
#endif
}

GetThreadsResult GetProcessThreads(int pid) {
  GetThreadsResult result;
  result.code = GET_THREADS_SUCCESS;

#ifdef _WIN32
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
  if (!hProcess) {
    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED) {
      result.code = GET_THREADS_PERMISSION_DENIED;
    } else {
      result.code = GET_THREADS_OPEN_PROCESS_FAILED;
    }
    return result;
  }

  DWORD_PTR mask = 0, systemMask = 0;
  if (GetProcessAffinityMask(hProcess, &mask, &systemMask)) {
    for (int i = 0; i < (int)(sizeof(DWORD_PTR) * 8); ++i) {
      if (mask & (1ULL << i)) {
        result.threads.push_back(i);
      }
    }
  } else {
    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED) {
      result.code = GET_THREADS_PERMISSION_DENIED;
    } else {
      result.code = GET_THREADS_QUERY_FAILED;
    }
  }

  CloseHandle(hProcess);

#else
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);

  if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpuSet) == 0) {
    for (int i = 0; i < CPU_SETSIZE; ++i) {
      if (CPU_ISSET(i, &cpuSet)) {
        result.threads.push_back(i);
      }
    }
  } else {
    if (errno == EPERM) {
      result.code = GET_THREADS_PERMISSION_DENIED;
    } else {
      result.code = GET_THREADS_QUERY_FAILED;
    }
  }
#endif

  return result;
}

} // namespace scheduler
