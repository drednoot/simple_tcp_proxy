#ifndef TCPPROXY_SERVER_H_
#define TCPPROXY_SERVER_H_

#include "logger.h"
#include "poll.h"
#include "socket.h"
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define kLocalhost "127.0.0.1"
#define kDefaultPort "1777"
#define kDefaultBufferSize 1024
#define kPollTimeout 5000
#define kDefaultSleepTime 1
#define kDefaultLogFilepath "./"
#define kDefaultFLogilename "log"

namespace Proxy {

class Server {
public:
  Server() noexcept;
  ~Server() noexcept;

  void Run(const std::string &connection_ip, const std::string &connection_port,
           const std::string &server_port = kDefaultPort);
  void AddLogger(const std::string &filepath = kDefaultLogFilepath,
                 const std::string &filename = kDefaultFLogilename);

private:
  void MainLoop();
  void ListenForConnectionLoop();

  int ResendFromFd(int fd);
  int SendToFd(int fd, std::string msg);
  void EraseFd(int fd);
  void Log(Logger::Level level, const std::string &msg);

  std::vector<struct pollfd> fds_;
  std::unordered_map<int, int> associations_;

  std::thread connections_;
  Socket server_;
  bool stop_ = false;
  std::string connection_ip_, connection_port_;
  std::mutex fds_change_;
  Logger *logger_;
};

} // namespace Proxy

#endif // TCPPROXY_SERVER_H_
