#ifndef TCPPROXY_LOGGER_H_
#define TCPPROXY_LOGGER_H_

#include <fstream>
#include <mutex>
#include <string>

namespace Proxy {

class Logger {
public:
  enum class Level {
    kInfo,
    kError,
  };

  Logger(const std::string &filepath, const std::string &filename);
  ~Logger();
  void Log(Level level, const std::string &message);

private:
  Logger() noexcept = delete;

  void AddLevel(Level level);
  void AddTime();

  std::ofstream file_;
  std::mutex lock_;
};

} // namespace Proxy

#endif // TCPPROXY_LOGGER_H_
