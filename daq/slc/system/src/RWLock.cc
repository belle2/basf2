#include "daq/slc/system/RWLock.h"

using namespace Belle2;

RWLock::RWLock() throw() : m_lock() {}

RWLock::~RWLock() throw() {}

bool RWLock::init() throw()
{
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_rwlock_init(&m_lock, &attr);
  pthread_rwlockattr_destroy(&attr);
  return true;
}

bool RWLock::rdlock() throw()
{
  if (pthread_rwlock_rdlock(&m_lock) != 0) {
    return false;
  } else {
    return true;
  }
}

bool RWLock::wrlock() throw()
{
  if (pthread_rwlock_wrlock(&m_lock) != 0) {
    return false;
  }
  return true;
}

bool RWLock::unlock() throw()
{
  if (pthread_rwlock_unlock(&m_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

bool RWLock::destroy() throw()
{
  if (pthread_rwlock_destroy(&m_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

