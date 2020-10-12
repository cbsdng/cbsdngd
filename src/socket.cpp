#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <cbsdng/daemon/socket.h>


std::vector<Socket *> Socket::all;


Socket::Socket(const std::string &socket_path) : socketPath{socket_path}
{
  it = all.emplace(all.end(), this);
  struct sockaddr_un addr;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    perror("socket error");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.data(), sizeof(addr.sun_path) - 1);
  unlink(socketPath.data());

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    std::cerr << "bind error: " << strerror(errno) << '\n';
    exit(1);
  }
  chmod(socketPath.data(), 0666);
  if (listen(fd, 5) == -1)
  {
    std::cerr << "listen error: " << strerror(errno) << '\n';
    exit(1);
  }
}


Socket::~Socket()
{
  cleanup();
  // all.erase(it);
}


void Socket::cleanup()
{
  unlink(socketPath.data());
  if (fd != -1)
  {
    close(fd);
    fd = -1;
  }
}


int Socket::waitForClient()
{
  int client;
  if ((client = accept(fd, NULL, NULL)) == -1)
  {
    std::cerr << "accept error: " << strerror(errno) << '\n';
    cleanup();
    return -1;
  }
  return client;
}
