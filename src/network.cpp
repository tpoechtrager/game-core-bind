#include "network.h"
#include <string>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace network {

bool Init() {
#ifdef _WIN32
  WSADATA wsaData;
  return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
  return true;
#endif
}

void Deinit() {
#ifdef _WIN32
  WSACleanup();
#endif
}

bool SendUdpMessage(const std::string& host, int port, const std::string& message) {
  auto sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return false;
  }

  sockaddr_in addr = { };
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<uint16_t>(port));
  addr.sin_addr.s_addr = inet_addr(host.c_str());

  if (addr.sin_addr.s_addr == INADDR_NONE) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    return false;
  }

  sendto(sock, message.c_str(), static_cast<int>(message.size()), 0, (sockaddr*)&addr, sizeof(addr));

#ifdef _WIN32
  closesocket(sock);
#else
  close(sock);
#endif

  return true;
}

} // namespace network
