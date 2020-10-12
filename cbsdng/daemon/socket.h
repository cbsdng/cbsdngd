#pragma once

#include <string>
#include <vector>

class Socket
{
public:
  Socket(const std::string &socketPath);
  ~Socket();

  static std::vector<Socket *> all;

  void cleanup();
  int waitForClient();

protected:
  int fd;
  std::string socketPath;
  std::vector<Socket *>::iterator it;
};
