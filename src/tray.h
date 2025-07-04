#pragma once

#ifdef _WIN32
#include <windows.h>
#else
typedef void* HMENU;
#endif

namespace tray {

void Init(const char* tooltip);
void AddMenuItem(const char* text, int id);
void AddCheckableMenuItem(const char* text, int id, bool checked);
void SetMenuItemChecked(int id, bool checked);
bool IsMenuItemChecked(int id);
void AddMenuItemToSubMenu(HMENU submenu, const char* text, int id);
void AddSubMenu(const char* text, HMENU submenu);
HMENU CreateSubMenu();
void PollTrayMessages();
void Destroy();

} // namespace tray
