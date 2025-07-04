#pragma once
#include <vector>

namespace scheduler {

// Return codes for binding a process to threads
enum BindResult {
  BIND_SUCCESS = 0,
  BIND_INVALID_THREAD_INDEX = 1,
  BIND_OPEN_PROCESS_FAILED = 2,
  BIND_PERMISSION_DENIED = 3,
  BIND_SETAFFINITY_FAILED = 4
};

// Return codes for querying process affinity
enum GetThreadsCode {
  GET_THREADS_SUCCESS = 0,
  GET_THREADS_OPEN_PROCESS_FAILED = 1,
  GET_THREADS_QUERY_FAILED = 2,
  GET_THREADS_PERMISSION_DENIED = 3
};

struct GetThreadsResult {
  GetThreadsCode code;
  std::vector<int> threads;
};

// Binds given PID to specific OS thread IDs (zero-based)
BindResult BindProcessToThreads(int pid, const std::vector<int>& threads);

// Returns list of thread IDs the process is currently bound to, plus status
GetThreadsResult GetProcessThreads(int pid);

} // namespace scheduler
