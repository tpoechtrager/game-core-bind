#pragma once

namespace admin {

bool IsRunningAsAdmin();
[[noreturn]] void RelaunchAsAdmin();
void EnsureRunningAsAdmin();

} // namespace admin
