#pragma once
#include <string>
#include <vector>
#include <memory>

namespace window {

enum class WindowPosition {
  Default,
  Center
};

struct WindowConfig {
  std::string title;
  int width = 800;
  int height = 600;
  bool resizable = true;
  WindowPosition position = WindowPosition::Default;
  bool scrollableContent = false;
};

using WindowPtr = std::shared_ptr<struct Window>;

WindowPtr Create(const WindowConfig& config);
void Destroy(WindowPtr win);
void Show(WindowPtr win);
void PollEvents();

// Controls
int AddStatic(WindowPtr win, int x, int y, int width, int height, const std::string& text);
int AddGroupBox(WindowPtr win, int x, int y, int width, int height, const std::string& label);
int AddComboBox(WindowPtr win, int x, int y, int width, int height, const std::vector<std::string>& items);
int AddCheckBox(WindowPtr win, int x, int y, int width, int height, const std::string& label);
int AddEditBox(WindowPtr win, int x, int y, int width, int height, const std::string& text = "", bool disabled = false);
int AddButton(WindowPtr win, int x, int y, int width, int height, const std::string& label);

// Value access
void SetCheckBoxChecked(WindowPtr win, int id, bool checked);
bool GetCheckBoxChecked(WindowPtr win, int id);
void SetComboBoxSelectedIndex(WindowPtr win, int id, int index);
int GetComboBoxSelectedIndex(WindowPtr win, int id);
std::string GetEditBoxText(WindowPtr win, int id);

void DestroyAllWindows();

} // namespace window
