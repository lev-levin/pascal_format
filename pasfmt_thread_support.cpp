#include "pasfmt_thread_support.h"
#include <algorithm>

/***** CPasFmt_ThreadList *****/

CPasFmt_ThreadList::CPasFmt_ThreadList() :
  FThreadListAccess(std::make_shared<CMutexItem>())
{
}

/*Получение номера потока (начиная с 1) в списке зарегистрированных потоков*/
int CPasFmt_ThreadList::ThreadIdToInt(const std::thread::id ThreadId)
{
  CriticalSection access(*FThreadListAccess);
  auto threadRef = std::find(FThreadList.begin(), FThreadList.end(), ThreadId);
  if (threadRef == FThreadList.end()) threadRef = FThreadList.insert(FThreadList.end(), ThreadId);
  return threadRef - FThreadList.begin() + 1;
}
