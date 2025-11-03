#ifndef CPASFMT_THREADLIST_H
#define CPASFMT_THREADLIST_H

#include <vector>
#include <thread>
#include <memory>
#include "dwsync.h"

class CPasFmt_ThreadList
{
public:
  CPasFmt_ThreadList();
private:
  std::vector<std::thread::id> FThreadList;      //список зарегистрированных потоков
  std::shared_ptr<CMutexItem> FThreadListAccess; //управление доступом к списку зарегистрированных потоков
public:
  /*Получение номера потока (начиная с 1) в списке зарегистрированных потоков*/
  int ThreadIdToInt(const std::thread::id ThreadId);
};

#endif // CPASFMT_THREADLIST_H
