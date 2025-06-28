#pragma once
#include <string>

namespace network {

bool Init();
void Deinit();
bool SendUdpMessage(const std::string& host, int port, const std::string& message);

} // namespace network
