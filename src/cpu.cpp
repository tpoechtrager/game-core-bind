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

// Known AMD models with cores per CCD
struct AMDModel {
  const char* name;
  int coresPerCCD;
};

static constexpr std::array<AMDModel, 16> AMD_KnownCCDModels = {{
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

// Returns CCD core count for known AMD models
static constexpr int FindCoresPerCCD(const std::string& brandStr) {
  for (const auto& entry : AMD_KnownCCDModels)
    if (brandStr.find(entry.name) != std::string::npos)
      return entry.coresPerCCD;
  return 0;
}

#include "cpu-intel.h"

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
  const int hwThreads = static_cast<int>(std::thread::hardware_concurrency());

  info.threads = hwThreads;
  info.isAMD   = brandStr.find("AMD ") != std::string::npos;
  info.isIntel = brandStr.find("Intel") != std::string::npos;

  // Helper to assign a CCD
  auto setCCD = [&](int index, bool isX3D, bool isEfficiency, bool isLowPowerEfficiency, int cores, int threads, int firstThread) {
    info.ccds[index].isX3D                = isX3D;
    info.ccds[index].isEfficiency         = isEfficiency;
    info.ccds[index].isLowPowerEfficiency = isLowPowerEfficiency;
    info.ccds[index].cores                = cores;
    info.ccds[index].threads              = threads;
    info.ccds[index].firstThreadNum       = firstThread;
    info.ccds[index].lastThreadNum        = firstThread + threads - 1;
  };

  if (info.isAMD) {
    const bool isX3D = brandStr.find("X3D") != std::string::npos;
    const int coresPerCCD = FindCoresPerCCD(brandStr);

    if (coresPerCCD > 0) {
      int threadsPerCCD = coresPerCCD * 2;
      int currentThread = 0;
      setCCD(0, isX3D, false, false, coresPerCCD, threadsPerCCD, currentThread);
      info.numCcds = 1;
      currentThread += threadsPerCCD;
      if (hwThreads > threadsPerCCD) {
        setCCD(1, false, false, false, coresPerCCD, threadsPerCCD, currentThread);
        info.numCcds = 2;
      }
    } else {
      setCCD(0, isX3D, false, false, hwThreads / 2, hwThreads, 0);
      info.numCcds = 1;
    }

  } else if (info.isIntel) {
    const auto* model = FindIntelModel(brandStr);
    if (model) {
      int currentThread = 0;
      for (int i = 0; i < model->numCcds; ++i) {
        const auto& ccd = model->ccds[i];
        setCCD(i, false, ccd.isEfficiency, ccd.isLowPowerEfficiency, ccd.cores, ccd.threads, currentThread);
        currentThread += ccd.threads;
      }
      info.numCcds = model->numCcds;
    } else {
      fprintf(stderr, "Warning: CPU '%s' not in database. P/E/LP-core count is unknown. Using fallback.\n", brandStr.c_str());
      fprintf(stderr, "Info: Ignore this warning if you are using a CPU older than Alder Lake.\n");
      setCCD(0, false, true, false, hwThreads / 2, hwThreads, 0);
      info.numCcds = 1;
    }

  } else {
    setCCD(0, false, false, false, hwThreads, hwThreads, 0);
    info.numCcds = 1;
  }

  return info;
}

} // namespace cpu
