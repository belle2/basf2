#include "daq/slc/readout/MMutex.h"

#include <daq/slc/base/Debugger.h>
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
  Belle2::debug("%s:%d", __FILE__, __LINE__);
  set(addr);
  init();
  Belle2::debug("%s:%d", __FILE__, __LINE__);
  return true;
}

bool MMutex::init() throw()
{
  Belle2::debug("%s:%d", __FILE__, __LINE__);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(_mu, &attr);
  pthread_mutexattr_destroy(&attr);
  Belle2::debug("%s:%d", __FILE__, __LINE__);
  return true;
}

bool MMutex::set(void* addr) throw()
{
  _mu = (pthread_mutex_t*)addr;
  return true;
}

bool MMutex::lock() throw()
{
  if (pthread_mutex_lock(_mu) != 0) {
    return false;
  } else {
    return true;
  }
}

bool MMutex::trylock() throw()
{
  if (pthread_mutex_lock(_mu) != 0) {
    return false;
  }
  return true;
}

bool MMutex::unlock() throw()
{
  if (pthread_mutex_unlock(_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

bool MMutex::destroy() throw()
{
  if (pthread_mutex_destroy(_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

const MMutex& MMutex::operator=(const MMutex& mutex) throw()
{
  _mu = mutex._mu;
  return *this;
}
