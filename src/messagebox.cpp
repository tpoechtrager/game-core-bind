#include "messagebox.h"
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace messagebox {

static HWND CreateTopmostWindow() {
#ifdef _WIN32
  return CreateWindowExA(
    WS_EX_TOPMOST,
    "STATIC", "GCBMessage",
    WS_POPUP,
    0, 0, 0, 0,
    nullptr, nullptr, GetModuleHandle(nullptr), nullptr
  );
#else
  return nullptr;
#endif
}

void Show(const std::string& title, const std::string& text) {
#ifdef _WIN32
  HWND dummy = CreateTopmostWindow();
  SetWindowPos(dummy, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  MessageBoxA(dummy, text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
  DestroyWindow(dummy);
#else
  printf("[MessageBox] %s: %s\n", title.c_str(), text.c_str());
#endif
}

bool ConfirmYesNo(const std::string& title, const std::string& text) {
#ifdef _WIN32
  HWND dummy = CreateTopmostWindow();
  SetWindowPos(dummy, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  int result = MessageBoxA(dummy, text.c_str(), title.c_str(), MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
  DestroyWindow(dummy);
  return result == IDYES;
#else
  printf("[Confirm] %s: %s [yes/no not implemented, defaulting to yes]\n", title.c_str(), text.c_str());
  return true;
#endif
}

} // namespace messagebox
