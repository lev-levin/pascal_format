#include "dwmutex.h"
#include <Windows.h>

/***** CMutexItem *****/

CMutexItem::CMutexItem() :
  m_MutexData(new HANDLE)
{
  *(HANDLE*)m_MutexData = CreateMutex(NULL, false, NULL);
}

CMutexItem::~CMutexItem()
{
  CloseHandle(*(HANDLE*)m_MutexData);
  delete (HANDLE*)m_MutexData;
}

/***** CMutexLocker *****/

CMutexLocker::CMutexLocker(CMutexItem& Mutex) :
  m_MutexRef(Mutex.m_MutexData)
{
  WaitForSingleObject(*(HANDLE*)m_MutexRef, INFINITE);
}

CMutexLocker::~CMutexLocker()
{
  ReleaseMutex(*(HANDLE*)m_MutexRef);
}
