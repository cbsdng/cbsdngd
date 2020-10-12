#pragma once
#include <cbsdng/message.h>

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

class AsyncWorker
{
public:
  AsyncWorker(const int &client);
  ~AsyncWorker();

  static std::list<AsyncWorker *> finished;
  static void removeFinished();
  static void terminate();
  static void wait();

  void process();
  void cleanup();
  void execute(const Message &m);

protected:
  int client;
  std::thread t;
  static std::mutex mutex;
  static std::condition_variable condition;
  static bool quit;

private:
  void _process();
};
