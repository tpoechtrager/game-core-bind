#include "display.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>
#include <string>


namespace display {

std::vector<MonitorInfo> GetMonitors() {
  std::vector<MonitorInfo> monitors;

#ifdef _WIN32
  DISPLAY_DEVICE device = { };
  device.cb = sizeof(device);

  for (int i = 0; EnumDisplayDevices(nullptr, i, &device, 0); ++i) {

    MonitorInfo info;
    info.deviceName = device.DeviceName;
    info.deviceString = device.DeviceString;
    info.isActive = (device.StateFlags & DISPLAY_DEVICE_ACTIVE) != 0;
    info.isPrimary = (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
    info.monitorName = "";

    info.posX = 0;
    info.posY = 0;
    info.width = 0;
    info.height = 0;
    info.bitsPerPel = 32;
    info.displayFrequency = 60;

    // Optional: Monitor-Beschreibung holen
    DISPLAY_DEVICE monitor = { };
    monitor.cb = sizeof(monitor);
    if (EnumDisplayDevices(device.DeviceName, 0, &monitor, 0)) {
      info.monitorName = monitor.DeviceString;
    }

    // Versuche aktuelle Settings lesen
    DEVMODE devmode = { };
    devmode.dmSize = sizeof(devmode);

    if (EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &devmode)) {
      info.posX = devmode.dmPosition.x;
      info.posY = devmode.dmPosition.y;
      info.width = devmode.dmPelsWidth;
      info.height = devmode.dmPelsHeight;
      info.bitsPerPel = devmode.dmBitsPerPel;
      info.displayFrequency = devmode.dmDisplayFrequency;
    }

    monitors.push_back(info);
  }
#endif

  return monitors;
}

bool DisableMonitor(const std::string& deviceName) {
#ifdef _WIN32
  DEVMODE devmode = { };
  devmode.dmSize = sizeof(devmode);
  devmode.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;
  devmode.dmPelsWidth = 0;
  devmode.dmPelsHeight = 0;
  devmode.dmPosition.x = 0;
  devmode.dmPosition.y = 0;

  LONG res = ChangeDisplaySettingsEx(deviceName.c_str(), &devmode, nullptr, CDS_UPDATEREGISTRY | CDS_GLOBAL, nullptr);
  return res == DISP_CHANGE_SUCCESSFUL;
#else
  return false;
#endif
}

bool EnableMonitor(const MonitorInfo& info) {
#ifdef _WIN32
  DEVMODE devmode = { };
  devmode.dmSize = sizeof(devmode);

  devmode.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

  devmode.dmPosition.x = info.posX;
  devmode.dmPosition.y = info.posY;
  devmode.dmPelsWidth = info.width;
  devmode.dmPelsHeight = info.height;
  devmode.dmBitsPerPel = info.bitsPerPel;
  devmode.dmDisplayFrequency = info.displayFrequency;

  LONG res = ChangeDisplaySettingsEx(info.deviceName.c_str(), &devmode, nullptr, CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);
  if (res != DISP_CHANGE_SUCCESSFUL) {
    return false;
  }

  // Globales Layout anwenden
  res = ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, 0, nullptr);
  return res == DISP_CHANGE_SUCCESSFUL;
#else
  return false;
#endif
}

} // namespace display
