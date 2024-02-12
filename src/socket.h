#ifndef TCPPROXY_SOCKET_H_
#define TCPPROXY_SOCKET_H_

#include <netdb.h>
#include <poll.h>
#include <string>

#define kDefaultBacklog 10

namespace Proxy {

class Socket {
public:
  enum class Status {
    kUninitialized,
    kCreated,
    kBound,
    kConnected,
    kListening,
  };

  // main contents
  Socket() noexcept {};
  ~Socket() noexcept;

  struct addrinfo GetAddrInfo() noexcept;
  int Create(const std::string &ip, const std::string &port) noexcept;
  int Bind() noexcept;
  int Listen() noexcept;
  int Connect() noexcept;
  int Accept() noexcept;

  int GetFd() noexcept;

  static Socket SetupServer(const std::string &ip,
                            const std::string &port) noexcept;
  static Socket SetupClient(const std::string &ip,
                            const std::string &port) noexcept;

private:
  int sockfd_ = -1;
  struct addrinfo info_ = {};
  Status status_ = Status::kUninitialized;
  int yes_ = 1;
};

} // namespace Proxy

#endif // TCPPROXY_SOCKET_H_
