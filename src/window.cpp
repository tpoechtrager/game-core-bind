#include "window.h"
#include "lua.h"

#ifdef _WIN32
#include <windows.h>
#include <vector>
#include <string>

namespace window {

struct Window {
  HWND hwnd;
  HWND contentHwnd;
  std::vector<HWND> controls;
  bool scrollable = false;
  int scrollOffsetY = 0;
  int contentHeight = 0;
};

static std::vector<WindowPtr> allWindows;
static const char* CLASS_NAME = "GCB_Window";

static DWORD GetWindowStyle(const WindowConfig& config) {
  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  if (config.resizable) style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
  if (config.scrollableContent) style |= WS_VSCROLL;
  return style;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  Window* win = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (!win) return DefWindowProc(hwnd, msg, wParam, lParam);

  switch (msg) {
    case WM_MOUSEWHEEL: {
      int delta = GET_WHEEL_DELTA_WPARAM(wParam);
      int direction = delta > 0 ? SB_LINEUP : SB_LINEDOWN;
      SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(direction, 0), 0);
      return 0;
    }

    case WM_VSCROLL: {
      SCROLLINFO si = {};
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_VERT, &si);

      switch (LOWORD(wParam)) {
        case SB_LINEUP:     si.nPos -= 60; break;
        case SB_LINEDOWN:   si.nPos += 60; break;
        case SB_PAGEUP:     si.nPos -= si.nPage; break;
        case SB_PAGEDOWN:   si.nPos += si.nPage; break;
        case SB_THUMBTRACK: si.nPos  = HIWORD(wParam); break;
        default: return 0;
      }

      si.fMask = SIF_POS;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
      GetScrollInfo(hwnd, SB_VERT, &si);

      int delta = win->scrollOffsetY - si.nPos;
      win->scrollOffsetY = si.nPos;

      ScrollWindow(win->contentHwnd, 0, delta, nullptr, nullptr);
      UpdateWindow(win->contentHwnd);
      return 0;
    }

  case WM_KEYDOWN:
    switch (wParam) {
      case VK_PRIOR: // Page Up
        SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
        return 0;
      case VK_NEXT: // Page Down
        SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
        return 0;
    }
    break;

    case WM_CLOSE: {
      lua::TriggerWindowCloseEvent(win);
      return 0;
    }
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK ContentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  Window* win = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (!win) return DefWindowProc(hwnd, msg, wParam, lParam);

  switch (msg) {
    case WM_COMMAND: {
      int id = LOWORD(wParam);
      lua::TriggerWindowEvent(win, id);
      return 0;
    }
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

static const char* CONTAINER_CLASS = "GCB_Container";

static void RegisterContainerWindowClass(HINSTANCE hInstance) {
  static bool registered = false;
  if (registered) return;

  WNDCLASSA wc = {};
  wc.lpfnWndProc = ContentProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CONTAINER_CLASS;
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  RegisterClassA(&wc);
  registered = true;
}

WindowPtr Create(const WindowConfig& config) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);

  static bool registered = false;
  if (!registered) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassA(&wc);
    registered = true;
  }

  if (config.scrollableContent) {
    RegisterContainerWindowClass(hInstance);
  }

  DWORD style = GetWindowStyle(config);
  RECT rect = { 0, 0, config.width, config.height };
  AdjustWindowRect(&rect, style, FALSE);

  int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
  if (config.position == WindowPosition::Center) {
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    x = (sw - (rect.right - rect.left)) / 2;
    y = (sh - (rect.bottom - rect.top)) / 2;
  }

  HWND hwnd = CreateWindowExA(
    0, CLASS_NAME, config.title.c_str(), style,
    x, y, rect.right - rect.left, rect.bottom - rect.top,
    nullptr, nullptr, hInstance, nullptr
  );
  if (!hwnd) return nullptr;

  auto win = std::make_shared<Window>();
  win->hwnd = hwnd;
  win->scrollable = config.scrollableContent;

  win->contentHwnd = win->scrollable
    ? CreateWindowExA(0, CONTAINER_CLASS, "", WS_CHILD | WS_VISIBLE,
        0, 0, config.width, 2000, hwnd, nullptr, hInstance, nullptr)
    : hwnd;

  if (win->contentHwnd != win->hwnd) {
    SetWindowLongPtr(win->contentHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win.get()));
  }

  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win.get()));
  allWindows.push_back(win);
  return win;
}

void Destroy(WindowPtr win) {
  if (!win) return;
  for (HWND h : win->controls) DestroyWindow(h);
  if (win->contentHwnd && win->contentHwnd != win->hwnd)
    DestroyWindow(win->contentHwnd);
  if (win->hwnd) DestroyWindow(win->hwnd);
  win->controls.clear();
  win->hwnd = nullptr;
}

void Show(WindowPtr win) {
  if (win && win->hwnd) {
    ShowWindow(win->hwnd, SW_SHOW);
    UpdateWindow(win->hwnd);
  }
}

void PollEvents() {
  MSG msg;
  while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

static void UpdateScroll(WindowPtr win, int newBottom) {
  if (!win->scrollable) return;
  if (newBottom <= win->contentHeight) return;

  win->contentHeight = newBottom;
  SCROLLINFO si = {};
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_RANGE | SIF_PAGE;
  si.nMin = 0;
  si.nMax = newBottom;
  si.nPage = 400;
  SetScrollInfo(win->hwnd, SB_VERT, &si, TRUE);
}

static int AddControl(WindowPtr win, const char* cls, const std::string& text, DWORD style,
                      int x, int y, int width, int height) {
  int id = static_cast<int>(win->controls.size());

  HWND h = CreateWindowExA(
    0, cls, text.c_str(),
    WS_CHILD | WS_VISIBLE | style,
    x, y + win->scrollOffsetY, width, height,
    win->contentHwnd,
    (HMENU)(INT_PTR)id,
    GetModuleHandle(nullptr),
    nullptr
  );

  win->controls.push_back(h);
  UpdateScroll(win, y + height + 10);
  return id;
}


int AddStatic(WindowPtr win, int x, int y, int width, int height, const std::string& text) {
  return AddControl(win, "STATIC", text, SS_LEFT, x, y, width, height);
}

int AddGroupBox(WindowPtr win, int x, int y, int width, int height, const std::string& label) {
  return AddControl(win, "BUTTON", label, BS_GROUPBOX, x, y, width, height);
}

int AddComboBox(WindowPtr win, int x, int y, int width, int height, const std::vector<std::string>& items) {
  int id = static_cast<int>(win->controls.size());

  HWND h = CreateWindowExA(
    0, "COMBOBOX", "",
    CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
    x, y + win->scrollOffsetY, width, height,
    win->contentHwnd,
    (HMENU)(INT_PTR)id,
    GetModuleHandle(nullptr),
    nullptr
  );

  for (const auto& item : items) {
    SendMessageA(h, CB_ADDSTRING, 0, (LPARAM)item.c_str());
  }
  SendMessageA(h, CB_SETCURSEL, 0, 0);
  win->controls.push_back(h); 
  UpdateScroll(win, y + height + 10);
  return static_cast<int>(win->controls.size() - 1);
}

int AddCheckBox(WindowPtr win, int x, int y, int width, int height, const std::string& label) {
  return AddControl(win, "BUTTON", label, WS_TABSTOP | BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE, x, y, width, height);
}

int AddEditBox(WindowPtr win, int x, int y, int width, int height, const std::string& text, bool disabled) {
  DWORD style = ES_LEFT | ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP;
  if (disabled) {
    style |= WS_DISABLED;
  }
  return AddControl(win, "EDIT", text, style, x, y, width, height);
}

int AddButton(WindowPtr win, int x, int y, int width, int height, const std::string& label) {
  return AddControl(win, "BUTTON", label, BS_PUSHBUTTON, x, y, width, height);
}

void SetCheckBoxChecked(WindowPtr win, int id, bool checked) {
  if (!win || id < 0 || id >= (int)win->controls.size()) return;
  SendMessageA(win->controls[id], BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool GetCheckBoxChecked(WindowPtr win, int id) {
  if (!win || id < 0 || id >= (int)win->controls.size()) return false;
  return SendMessageA(win->controls[id], BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void SetComboBoxSelectedIndex(WindowPtr win, int id, int index) {
  if (!win || id < 0 || id >= (int)win->controls.size()) return;
  SendMessageA(win->controls[id], CB_SETCURSEL, index, 0);
}

int GetComboBoxSelectedIndex(WindowPtr win, int id) {
  if (!win || id < 0 || id >= (int)win->controls.size()) return -1;
  return (int)SendMessageA(win->controls[id], CB_GETCURSEL, 0, 0);
}


std::string GetEditBoxText(WindowPtr win, int id) {
  if (!win || id < 0 || id >= (int)win->controls.size()) return "";
  char buf[512] = {};
  GetWindowTextA(win->controls[id], buf, sizeof(buf));
  return std::string(buf);
}

void DestroyAllWindows() {
  for (auto& win : allWindows) {
    Destroy(win);
  }
  allWindows.clear();
}

} // namespace window

#else
// Linux dummy
namespace window {
struct Window {};
WindowPtr Create(const WindowConfig&) { return std::make_shared<Window>(); }
void Destroy(WindowPtr) {}
void Show(WindowPtr) {}
void PollEvents() {}
int AddStatic(WindowPtr, int, int, int, int, const std::string&) { return 0; }
int AddGroupBox(WindowPtr, int, int, int, int, const std::string&) { return 0; }
int AddComboBox(WindowPtr, int, int, int, int, const std::vector<std::string>&) { return 0; }
int AddCheckBox(WindowPtr, int, int, int, int, const std::string&) { return 0; }
int AddEditBox(WindowPtr, int, int, int, int, const std::string&, bool disabled) { return 0; }
int AddButton(WindowPtr, int, int, int, int, const std::string&) { return 0; }
void SetCheckBoxChecked(WindowPtr, int, bool) {}
bool GetCheckBoxChecked(WindowPtr, int) { return false; }
std::string GetEditBoxText(WindowPtr, int) { return ""; }
void SetComboBoxSelectedIndex(WindowPtr, int, int) {}
int GetComboBoxSelectedIndex(WindowPtr, int) { return -1; }
void DestroyAllWindows() { }
} // namespace window
#endif
