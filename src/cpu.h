#include <string>

namespace cpu {

constexpr int MAX_CCDS = 8;

// Represents one CCD (chiplet)
struct CCDInfo {
  bool isX3D;         // AMD only
  bool isEfficiency;  // Intel only
  bool isLowPowerEfficiency; // Intel only
  int cores;
  int threads;
  int firstThreadNum;
  int lastThreadNum;

  int threadsPerCore() const {
    return threads / cores;
  }
};

// Overall CPU info with per-CCD breakdown
struct CPUInfo {
  bool isAMD;
  bool isIntel;

  int threads;
  int numCcds;
  std::string name; // CPU brand string
  CCDInfo ccds[MAX_CCDS];
};

// Returns basic CPU topology info, including brand, thread count,
// number of CCDs, cores/threads per CCD, and X3D detection (AMD only).
CPUInfo GetCPUInfo();

} // namespace cpu
