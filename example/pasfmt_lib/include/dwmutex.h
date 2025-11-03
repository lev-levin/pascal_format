#ifndef CMUTEXLOCKER_H
#define CMUTEXLOCKER_H

class CMutexItem
{
public:
  CMutexItem();
  ~CMutexItem();
public:
  void* m_MutexData;
};

class CMutexLocker
{
public:
  CMutexLocker(CMutexItem& Mutex);
  ~CMutexLocker();
private:
  void* m_MutexRef;
};

#endif // CMUTEXLOCKER_H
