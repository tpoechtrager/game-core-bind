#include "console.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace console {

// Hides the console window (Windows only)
void HideWindow() {
#ifdef _WIN32
  FreeConsole();
#endif
}

} // namespace console
