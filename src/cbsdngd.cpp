#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <cbsdng/daemon/asyncworker.h>
#include <cbsdng/daemon/socket.h>


Socket *s;


void signalHandler(int sig)
{
  signal(sig, SIG_IGN);
  AsyncWorker::terminate();
  AsyncWorker::wait();
  s->cleanup();
  exit(0);
}


void ignoreSignal(int sig)
{
  signal(sig, SIG_IGN);
}

int main(int argc, char **argv)
{
  const auto optstr = "ds:";
  bool d = false;
  std::string socketPath = "/var/run/cbsdng/cbsdng.sock";
  int c;
  int optreset = 1;
  int optind = 1;

  signal(SIGINT, signalHandler);
  signal(SIGPIPE, ignoreSignal);

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
      case 's':
        socketPath = optarg;
        break;
      case 'd':
        d = true;
        break;
      default:
        return 1;
    }
  }

  if (d)
  {
    daemon(0, 0);
  }
  s = new Socket(socketPath);

  while (1)
  {
    auto client = s->waitForClient();
    if (client != -1)
    {
      new AsyncWorker(client);
    }
    else
    {
      std::cerr << "Error accepting client!\n";
    }
  }
  delete s;
  return 0;
}
