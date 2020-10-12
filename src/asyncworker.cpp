#include <cbsdng/daemon/asyncworker.h>

#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>

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
  while (!quit)
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
  }
}

void AsyncWorker::execute(const Message &m)
{
  std::cout << m.data() << std::endl;
  switch(m.gettype())
  {
    case 0:
    {
      std::cout << "Executing cbsd j" << m.getpayload() << std::endl;
      std::string command = "cbsd j" + m.getpayload();
      system(command.data());
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
