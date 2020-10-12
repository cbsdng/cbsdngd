#include <cbsdng/daemon/asyncworker.h>
#include <cbsdng/daemon/socket.h>

#include <iostream>
#include <signal.h>

Socket s("/tmp/cbsd.sock");

void signalHandler(int sig)
{
  signal(sig, SIG_IGN);
  AsyncWorker::terminate();
  AsyncWorker::wait();
  s.cleanup();
  exit(0);
}

int main()
{
  signal(SIGINT, signalHandler);

  while (1)
  {
    auto client = s.waitForClient();
    if (client != -1)
    {
      new AsyncWorker(client);
    }
    else
    {
      std::cerr << "Error accepting client!" << std::endl;
    }
  }
  return 0;
}
