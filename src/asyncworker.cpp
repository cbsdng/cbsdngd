#include <iostream>
#include <vector>
#include <signal.h>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/event.h>
#include <chrono>

#include <cbsdng/daemon/asyncworker.h>


#define READ_END 0
#define WRITE_END 1


bool AsyncWorker::quit = false;
std::mutex AsyncWorker::mutex;
std::condition_variable AsyncWorker::condition;
std::list<AsyncWorker *> AsyncWorker::finished;
static auto finishedThread = std::thread(&AsyncWorker::removeFinished);


AsyncWorker::AsyncWorker(const int &cl) : client{cl}
{
  t = std::thread(&AsyncWorker::_process, this);
}


AsyncWorker::~AsyncWorker()
{
  t.join();
  cleanup();
}


void AsyncWorker::cleanup()
{
  if (client != -1)
  {
    close(client);
    client = -1;
  }
}


void AsyncWorker::process()
{
  int rc;
  char buf[128];
  std::stringstream raw_data;

  rc = read(client, buf, sizeof(buf));
  if (rc < 0)
  {
    perror("read");
    cleanup();
    return;
  }
  else if (rc == 0)
  {
    cleanup();
    return;
  }
  else if (rc != sizeof(buf))
  {
    buf[rc] = '\0';
  }
  raw_data << buf;
  int size;
  int id;
  int type;
  std::string data;
  raw_data >> size >> id >> type;
  if (raw_data.fail())
  {
    return;
  }
  while(!raw_data.eof())
  {
    if (data.size() != 0)
    {
      data += ' ';
    }
    std::string s;
    raw_data >> s;
    data += s;
  }
  Message m(id, type, data);
  execute(m);
  close(client);
}


void AsyncWorker::execute(const Message &m)
{
  switch(m.gettype())
  {
    case 0:
    {
      int childOut[2];
      int childErr[2];
      if (pipe(childOut) == -1)
      {
        std::cerr << "Failed to initialize output pipe\n";
        return;
      }
      if (pipe(childErr) == -1)
      {
        std::cerr << "Failed to initialize error pipe\n";
        return;
      }

      auto pid = fork();
      if (pid < 0)
      {
        std::cerr << "Failed to fork()\n";
        return;
      }
      else if (pid == 0) // child
      {
        close(childOut[READ_END]);
        close(childErr[READ_END]);
        if (dup2(childOut[WRITE_END], STDOUT_FILENO) == -1)
        {
          std::cerr << "Failed to redirect output\n";
          exit(1);
        }
        if (dup2(childErr[WRITE_END], STDERR_FILENO) == -1)
        {
          std::cerr << "Failed to redirect error\n";
          exit(1);
        }
        std::string command = "j" + m.getpayload();
        std::string raw_command = "cbsd " + command;
        std::vector<char *> args;
        char *token = strtok(raw_command.data(), " ");
        args.push_back(token);
        while ((token = strtok(nullptr, " ")) != nullptr)
        {
          args.push_back(token);
        }
        execvp(args[0], args.data());
      }
      else // parent
      {
        close(childOut[WRITE_END]);
        close(childErr[WRITE_END]);
        int r;
        struct kevent events[2];
        struct kevent tevent;
        int kq = kqueue();
        if (kq == -1)
        {
          std::cerr << "kqueue: \n";
        }
        EV_SET(events, childOut[READ_END], EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_READ, 0, nullptr);
        EV_SET(events+1, childErr[READ_END], EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_READ, 0, nullptr);
        int ret = kevent(kq, events, 2, nullptr, 0, nullptr);
        if (ret == -1)
        {
          std::cerr << "kevent register: " << strerror(errno) << '\n';
        }
        while (true)
        {
          int ret = kevent(kq, nullptr, 0, &tevent, 1, nullptr);
          if (ret == -1 || tevent.data == 0) { break; }
          char buffer[tevent.data+1];
          r = read(tevent.ident, buffer, tevent.data);
          if (r <= 0) { continue; }
          buffer[r] = '\0';
          Message m;
          m.data(0, 0, buffer);
          const auto &rawData = m.data();
          write(client, rawData.data(), rawData.size());
        }
        int st;
        waitpid(pid, &st, 0);
      }
      break;
    }
    default:
      break;
  }
}

void AsyncWorker::_process()
{
  process();
  {
    std::unique_lock<std::mutex> lock(mutex);
    finished.push_back(this);
  }
  condition.notify_one();
}


void AsyncWorker::removeFinished()
{
  while (1)
  {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [] { return !finished.empty(); });
    auto worker = finished.front();
    finished.pop_front();
    if (worker != nullptr)
    {
      delete worker;
    }
    if (quit && finished.empty())
    {
      return;
    }
  }
}


void AsyncWorker::terminate()
{
  quit = true;
  {
    std::unique_lock<std::mutex> lock(mutex);
    finished.push_back(nullptr);
  }
  condition.notify_all();
}


void AsyncWorker::wait() { finishedThread.join(); }
