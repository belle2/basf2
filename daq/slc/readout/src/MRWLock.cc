#include "daq/slc/readout/MRWLock.h"

using namespace Belle2;

MRWLock::MRWLock() throw() : _lock(NULL) {}

MRWLock::MRWLock(const MRWLock& RWLock) throw()
{
  *this = RWLock;
}

MRWLock::MRWLock(void* addr) throw()
{
  set((pthread_rwlock_t*)addr);
}

MRWLock::~MRWLock() throw() {}

bool MRWLock::init(void* addr) throw()
{
  set(addr);
  init();
  return true;
}

bool MRWLock::init() throw()
{
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_rwlock_init(_lock, &attr);
  pthread_rwlockattr_destroy(&attr);
  return true;
}

bool MRWLock::set(void* addr) throw()
{
  _lock = (pthread_rwlock_t*)addr;
  return true;
}

bool MRWLock::rdlock() throw()
{
  if (pthread_rwlock_rdlock(_lock) != 0) {
    return false;
  } else {
    return true;
  }
}

bool MRWLock::wrlock() throw()
{
  if (pthread_rwlock_wrlock(_lock) != 0) {
    return false;
  }
  return true;
}

bool MRWLock::unlock() throw()
{
  if (pthread_rwlock_unlock(_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

bool MRWLock::destroy() throw()
{
  if (pthread_rwlock_destroy(_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

const MRWLock& MRWLock::operator=(const MRWLock& lock) throw()
{
  _lock = lock._lock;
  return *this;
}
