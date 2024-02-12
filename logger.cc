#include "logger.h"
#include <ctime>
#include <iomanip>

namespace Proxy {

Logger::Logger(const std::string &filepath, const std::string &filename)
    : file_(filepath + filename) {}

Logger::~Logger() { file_.close(); }

void Logger::Log(Level level, const std::string &message) {
  lock_.lock();
  if (!file_.is_open())
    return;

  AddLevel(level);
  AddTime();
  file_ << ": " << message << std::endl;
  lock_.unlock();
}

void Logger::AddLevel(Level level) {
  if (!file_.is_open())
    return;

  switch (level) {
  case Level::kInfo:
    file_ << "[INFO]";
    break;
  case Level::kError:
    file_ << "[ERROR]";
    break;
  default:
    file_ << "[?]";
  }
}

void Logger::AddTime() {
  if (!file_.is_open())
    return;

  auto now = std::time(nullptr);
  auto tm = *std::localtime(&now);

  file_ << "[" << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "]";
}

} // namespace Proxy
