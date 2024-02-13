#include "server.h"
#include "poll.h"
#include "socket.h"
#include <chrono>
#include <string>
#include <thread>
#include <unistd.h>

namespace Proxy {

Server::Server() noexcept {}
Server::~Server() noexcept {
  connections_.join();
  for (auto pfd : fds_) {
    close(pfd.fd);
  }
  if (logger_) {
    delete logger_;
    logger_ = nullptr;
  }
}

void Server::Run(const std::string &connection_ip,
                 const std::string &connection_port,
                 const std::string &server_port) {
  connection_ip_ = connection_ip;
  connection_port_ = connection_port;
  server_ = Socket::SetupServer(kLocalhost, server_port);
  if (server_.GetFd() == -1) {
    Log(Logger::Level::kError, "Couldn't connect to a server");
  } else {
    Log(Logger::Level::kInfo, "Connected to a server");
    connections_ = std::thread(&Server::ListenForConnectionLoop, this);
    Log(Logger::Level::kInfo, "Started listening for connections");
    MainLoop();
  }
}

void Server::MainLoop() {
  while (!stop_) {
    int count = poll(fds_.data(), fds_.size(), kPollTimeout);
    if (count == -1) {
      stop_ = true;
    } else if (count > 0) {
      for (size_t i = 0, checked = 0; i < fds_.size() && checked < count; ++i) {
        if (fds_[i].revents & POLLIN) {
          ResendFromFd(fds_[i].fd);

          ++checked;
        }
      }
      // std::this_thread::sleep_for(std::chrono::milliseconds(kDefaultSleepTime));
    }
  }
}

void Server::ListenForConnectionLoop() {
  while (!stop_) {
    int ds_fd = server_.Accept();
    int us_fd = Socket::SetupClient(connection_ip_, connection_port_).GetFd();
    if (ds_fd == -1 || us_fd == -1) {
      Log(Logger::Level::kError, "Couldn't connect user");
      continue;
    }

    Log(Logger::Level::kInfo,
        std::string("New user connected, fds: ") + std::to_string(ds_fd) +
            " (downstream) : " + std::to_string(us_fd) + " (upstream)");
    fds_change_.lock();
    associations_.insert({ds_fd, us_fd});
    associations_.insert({us_fd, ds_fd});

    fds_.push_back({ds_fd, POLLIN, 0});
    fds_.push_back({us_fd, POLLIN, 0});
    fds_change_.unlock();
  }
}

int Server::ResendFromFd(int fd) {
  std::string buf(kDefaultBufferSize, 0);
  int received = recv(fd, buf.data(), kDefaultBufferSize, 0);

  if (received == -1) {
    perror("recv");
    Log(Logger::Level::kError, std::string("Couldn't receive data from fd ") +
                                   std::to_string(fd) + ": " +
                                   std::to_string(errno));
    return errno;
  } else if (received == 0) {
    EraseFd(fd);
    Log(Logger::Level::kInfo,
        std::string("Fd closed connection: ") + std::to_string(fd));
    return -1;
  }

  int res = SendToFd(associations_.at(fd), buf.substr(0, received));
  if (!res) {
    return res;
  }

  return 0;
}

int Server::SendToFd(int fd, std::string msg) {
  int sent = send(fd, msg.data(), msg.length(), 0);

  if (sent == -1) {
    perror("send");
    Log(Logger::Level::kError, std::string("Couldn't send data to fd ") +
                                   std::to_string(fd) + ": " +
                                   std::to_string(errno));
    return errno;
  } else if (sent < msg.length()) {
    std::string remains = msg.substr(sent, msg.length() - sent);
    Log(Logger::Level::kInfo, std::string("Only sent ") + std::to_string(sent) +
                                  "/" + std::to_string(msg.length()) +
                                  " on fd " + std::to_string(fd));
    int res = SendToFd(fd, remains);
    return res;
  }

  Log(Logger::Level::kInfo, std::string("Sent from fd ") + std::to_string(fd) +
                                " message: \'" + msg + "\'");

  return 0;
}

void Server::EraseFd(int fd) {
  for (size_t i = 0; i < fds_.size(); ++i) {
    if (fds_[i].fd == fd) {
      fds_change_.lock();
      fds_.erase(fds_.begin() + i);
      close(fd);
      close(associations_.at(fd));
      associations_.erase(associations_.at(fd));
      associations_.erase(fd);
      fds_change_.unlock();
      break;
    }
  }
}

void Server::AddLogger(const std::string &filepath,
                       const std::string &filename) {
  if (!logger_)
    logger_ = new Logger(filepath, filename);
}

void Server::Log(Logger::Level level, const std::string &msg) {
  if (!logger_)
    return;
  logger_->Log(level, msg);
}

} // namespace Proxy
