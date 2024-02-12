#include "server.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cout << "Usage: proxy <proxy port> <server ip> <server port>"
              << std::endl;
    return 1;
  }

  Proxy::Server s;
  s.AddLogger();
  s.Run(argv[2], argv[3], argv[1]);

  return 0;
}
