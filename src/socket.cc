#include "socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Proxy {

int Socket::Create(const std::string &ip, const std::string &port) noexcept {
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *servinfo = nullptr;

  int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo);
  if (status != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    return status;
  }

  for (auto info = servinfo; info; info = info->ai_next) {
    int fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (fd != -1) {
      sockfd_ = fd;
      info_ = *info;
      break;
    } else {
      perror("socket");
    }
  }

  if (sockfd_ == -1) {
    std::cerr << "Socket couldn't be created\n";
    freeaddrinfo(servinfo);
    return errno;
  }

  freeaddrinfo(servinfo);
  status_ = Status::kCreated;
  return 0;
}

int Socket::Bind() noexcept {
  if (status_ != Status::kCreated)
    return -1;

  int status =
      setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &yes_, sizeof(yes_));
  if (status == -1) {
    perror("setsockopt");
    return errno;
  }

  status = bind(sockfd_, info_.ai_addr, info_.ai_addrlen);
  if (status == -1) {
    perror("bind");
    return errno;
  }

  status_ = Status::kBound;
  return 0;
}

int Socket::Listen() noexcept {
  if (status_ != Status::kBound)
    return -1;

  int status = listen(sockfd_, kDefaultBacklog);
  if (status == -1) {
    perror("listen");
    return errno;
  }

  status_ = Status::kListening;
  return 0;
}

int Socket::Connect() noexcept {
  if (status_ != Status::kCreated)
    return -1;

  int status = connect(sockfd_, info_.ai_addr, info_.ai_addrlen);
  if (status == -1) {
    perror("connect");
    return errno;
  }

  status_ = Status::kConnected;
  return 0;
}

int Socket::Accept() noexcept {
  if (status_ != Status::kListening)
    return -1;
  struct sockaddr_storage storage = {};
  socklen_t addr_size = 0;

  int fd = accept(sockfd_, (struct sockaddr *)&storage, &addr_size);
  if (fd == -1) {
    perror("accept");
    return -1;
  }

  return fd;
}

Socket Socket::SetupServer(const std::string &ip,
                           const std::string &port) noexcept {
  Socket sock;
  sock.Create(ip, port);
  sock.Bind();
  sock.Listen();
  return sock;
}

Socket Socket::SetupClient(const std::string &ip,
                           const std::string &port) noexcept {
  Socket sock;
  sock.Create(ip, port);
  sock.Connect();
  return sock;
}

int Socket::GetFd() noexcept { return sockfd_; }

Socket::~Socket() noexcept {
  if (status_ != Status::kUninitialized) {
    sockfd_ = -1;
    status_ = Status::kUninitialized;
  }
}

} // namespace Proxy
