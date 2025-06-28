#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace desktop {

#ifdef _WIN32

// Internal error check helper
static void CheckAndAbort(BOOL result) {
  if (!result) {
    std::cerr << "SystemParametersInfo failed, error: " << GetLastError() << std::endl;
    std::abort();
  }
}

void DisableEffects() {
  ANIMATIONINFO animationInfo = { sizeof(ANIMATIONINFO), 0 };

  CheckAndAbort(SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &animationInfo, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));

  BOOL enabled = FALSE;
  CheckAndAbort(SystemParametersInfo(SPI_SETMENUFADE, 0, &enabled, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));
  CheckAndAbort(SystemParametersInfo(SPI_SETCURSORSHADOW, 0, &enabled, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));
}

void EnableEffects() {
  ANIMATIONINFO animationInfo = { sizeof(ANIMATIONINFO), 1 };

  CheckAndAbort(SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &animationInfo, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));

  BOOL enabled = TRUE;
  CheckAndAbort(SystemParametersInfo(SPI_SETMENUFADE, 0, &enabled, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));
  CheckAndAbort(SystemParametersInfo(SPI_SETCURSORSHADOW, 0, &enabled, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE));
}

#else

// Dummy implementations for non-Windows platforms
void DisableEffects() {
  // No operation
}

void EnableEffects() {
  // No operation
}

#endif

} // namespace desktop
