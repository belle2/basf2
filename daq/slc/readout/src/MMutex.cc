#include "daq/slc/readout/MMutex.h"

using namespace Belle2;

MMutex::MMutex() throw() {}

MMutex::MMutex(const MMutex& mutex) throw()
{
  *this = mutex;
}

MMutex::MMutex(void* addr) throw()
{
  set((pthread_mutex_t*)addr);
}

MMutex::~MMutex() throw() {}

bool MMutex::init(void* addr) throw()
{
  set(addr);
  init();
  return true;
}

bool MMutex::init() throw()
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(m_mu, &attr);
  pthread_mutexattr_destroy(&attr);
  return true;
}

bool MMutex::set(void* addr) throw()
{
  m_mu = (pthread_mutex_t*)addr;
  return true;
}

bool MMutex::lock() throw()
{
  if (pthread_mutex_lock(m_mu) != 0) {
    return false;
  } else {
    return true;
  }
}

bool MMutex::trylock() throw()
{
  if (pthread_mutex_lock(m_mu) != 0) {
    return false;
  }
  return true;
}

bool MMutex::unlock() throw()
{
  if (pthread_mutex_unlock(m_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

bool MMutex::destroy() throw()
{
  if (pthread_mutex_destroy(m_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

const MMutex& MMutex::operator=(const MMutex& mutex) throw()
{
  m_mu = mutex.m_mu;
  return *this;
}
