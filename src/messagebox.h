#pragma once

#include <string>

namespace messagebox {

void Show(const std::string& title, const std::string& text);
bool ConfirmYesNo(const std::string& title, const std::string& text);

} // namespace messagebox
