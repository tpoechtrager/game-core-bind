#include "tray.h"
#include "lua.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstring>

namespace tray {

static NOTIFYICONDATAA nid;
static HMENU trayMenu = nullptr;
static HINSTANCE hInstance = nullptr;
static UINT WM_TRAY = WM_USER + 1;

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_TRAY) {
    if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
      POINT pt;
      GetCursorPos(&pt);
      SetForegroundWindow(hwnd);
      TrackPopupMenu(trayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    }
  } else if (msg == WM_COMMAND) {
    int cmd = LOWORD(wParam);
    lua::TriggerTrayEvent(cmd);
  } else if (msg == WM_DESTROY) {
    Shell_NotifyIconA(NIM_DELETE, &nid);
    PostQuitMessage(0);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Init(const char* tooltip) {
  hInstance = GetModuleHandle(nullptr);

  WNDCLASS wc;
  std::memset(&wc, 0, sizeof(wc));
  wc.lpfnWndProc = TrayWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "GCB_Tray";

  RegisterClass(&wc);
  HWND hwnd = CreateWindowEx(0, "GCB_Tray", "Tray", 0, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

  trayMenu = CreatePopupMenu();

  std::memset(&nid, 0, sizeof(nid));
  nid.cbSize = sizeof(NOTIFYICONDATAA);
  nid.hWnd = hwnd;
  nid.uID = 1;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAY;
  nid.hIcon = static_cast<HICON>(LoadIcon(nullptr, IDI_INFORMATION));
  strncpy(nid.szTip, tooltip, sizeof(nid.szTip) - 1);

  Shell_NotifyIconA(NIM_ADD, &nid);
}

void AddMenuItem(const char* text, int id) {
  AppendMenu(trayMenu, MF_STRING, id, text);
}

void AddCheckableMenuItem(const char* text, int id, bool checked) {
  UINT flags = MF_STRING;
  if (checked) {
    flags |= MF_CHECKED;
  } else {
    flags |= MF_UNCHECKED;
  }
  AppendMenu(trayMenu, flags, id, text);
}

void SetMenuItemChecked(int id, bool checked) {
  CheckMenuItem(trayMenu, id, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}

bool IsMenuItemChecked(int id) {
  return (GetMenuState(trayMenu, id, MF_BYCOMMAND) & MF_CHECKED) != 0;
}

void AddMenuItemToSubMenu(HMENU submenu, const char* text, int id) {
  AppendMenu(submenu, MF_STRING, id, text);
}

void AddSubMenu(const char* text, HMENU submenu) {
  AppendMenu(trayMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(submenu), text);
}

HMENU CreateSubMenu() {
  return CreatePopupMenu();
}

void PollTrayMessages() {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void Destroy() {
  if (nid.hWnd) {
    Shell_NotifyIconA(NIM_DELETE, &nid);
    DestroyWindow(nid.hWnd);
    nid.hWnd = nullptr;
  }
  if (trayMenu) {
    DestroyMenu(trayMenu);
    trayMenu = nullptr;
  }
}

} // namespace tray

#else

namespace tray {

void Init(const char*) {}
void AddMenuItem(const char*, int) {}
void AddCheckableMenuItem(const char*, int, bool) {}
void SetMenuItemChecked(int, bool) {}
bool IsMenuItemChecked(int) { return false; }
void AddMenuItemToSubMenu(HMENU, const char*, int) {}
void AddSubMenu(const char*, HMENU) {}
HMENU CreateSubMenu() { return nullptr; }
void PollTrayMessages() {}
void Destroy() {}

} // namespace tray

#endif
