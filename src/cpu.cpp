#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <array>
#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#endif

#include "cpu.h"

namespace cpu {

// Model name + cores per CCD (used for detection)
struct CCDModel {
  const char* name;
  int coresPerCCD;
};

// Known AMD models with 1–2 CCDs
static constexpr std::array<CCDModel, 16> AMD_KnownCCDModels = {{
  { "3950",     8 },
  { "5950",     8 },
  { "7950",     8 },
  { "9950",     8 },
  { "7945HX",   8 },
  { "8945HX",   8 },
  { "9955HX",   8 },
  { "Max+ 395", 8 },
  { "3900",     6 },
  { "5900",     6 },
  { "7900",     6 },
  { "9900",     6 },
  { "7845HX",   6 },
  { "8940HX",   6 },
  { "9850HX",   6 },
  { "Max 390",  6 }
}};

// Check if brand string contains known model
static constexpr int FindCoresPerCCD(const std::string& brandStr) {
  for (const auto& entry : AMD_KnownCCDModels)
    if (brandStr.find(entry.name) != std::string::npos)
      return entry.coresPerCCD;
  return 0;
}

CPUInfo GetCPUInfo() {
  char brand[49] = {};

#if defined(_WIN32)
  int cpuInfo[4];
  __cpuid(cpuInfo, 0x80000002);
  std::memcpy(brand, cpuInfo, sizeof(cpuInfo));
  __cpuid(cpuInfo, 0x80000003);
  std::memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));
  __cpuid(cpuInfo, 0x80000004);
  std::memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
  brand[48] = '\0';
#else
  FILE* f = std::fopen("/proc/cpuinfo", "r");
  if (f) {
    char line[256];
    while (std::fgets(line, sizeof(line), f)) {
      if (std::sscanf(line, "model name : %48[^\n]", brand) == 1) {
        break;
      }
    }
    std::fclose(f);
  }
#endif

  CPUInfo info{};
  info.name = brand;
  const std::string& brandStr = info.name;
  const int threads = static_cast<int>(std::thread::hardware_concurrency());

  info.threads = threads;
  info.isAMD   = brandStr.find("AMD ") != std::string::npos;
  info.isIntel = brandStr.find("Intel") != std::string::npos;

  // Helper to set CCD info
  auto setCCD = [&](int index, bool isX3D, int cores, int threads, int firstThread) {
    info.ccds[index].isX3D          = isX3D;
    info.ccds[index].cores          = cores;
    info.ccds[index].threads        = threads;
    info.ccds[index].firstThreadNum = firstThread;
    info.ccds[index].lastThreadNum  = firstThread + threads - 1;
  };

  if (info.isAMD) {
    const bool isX3D = brandStr.find("X3D") != std::string::npos;
    const int coresPerCcd = FindCoresPerCCD(brandStr);

    if (coresPerCcd > 0) {
      int threadsPerCcd = coresPerCcd * 2;
      int currentThread = 0;

      // CCD 0
      setCCD(0, isX3D, coresPerCcd, threadsPerCcd, currentThread);
      info.numCcds = 1;
      currentThread += threadsPerCcd;

      // CCD 1 (if available)
      if (threads > threadsPerCcd) {
        setCCD(1, false, coresPerCcd, threadsPerCcd, currentThread);
        info.numCcds = 2;
      }

    } else {
      // Unknown model, assume single CCD with threads/2 cores
      setCCD(0, isX3D, threads / 2, threads, 0);
      info.numCcds = 1;
    }

  } else if (info.isIntel) {
    // Assume Intel has no CCDs, but reuse structure
    setCCD(0, false, threads / 2, threads, 0);
    info.numCcds = 1;

  } else {
    // Unknown CPU, fallback
    setCCD(0, false, threads, threads, 0);
    info.numCcds = 1;
  }

  return info;
}

} // namespace cpu
