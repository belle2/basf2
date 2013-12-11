#include "daq/slc/readout/modules/MCond.h"

#include <sys/time.h>

using namespace Belle2;

MCond::MCond() throw() {}

MCond::MCond(const MCond& cond) throw()
{
  *this = cond;
}

MCond::MCond(void* addr) throw()
{
  set((pthread_cond_t*)addr);
}

MCond::~MCond() throw() {}

bool MCond::init(void* addr) throw()
{
  set(addr);
  init();
  return true;
}

bool MCond::init() throw()
{
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_cond_init(_cond, &attr);
  pthread_condattr_destroy(&attr);
  return true;
}

bool MCond::set(void* addr) throw()
{
  _cond = (pthread_cond_t*)addr;
  return true;
}

bool MCond::destroy() throw()
{
  pthread_cond_destroy(_cond);
  return true;
}

bool MCond::signal() throw()
{
  if (pthread_cond_signal(_cond) == 0) return true;
  else return false;
}

bool MCond::broadcast() throw()
{
  if (pthread_cond_broadcast(_cond) == 0) return true;
  else return false;
}

bool MCond::wait(MMutex& cond) throw()
{
  if (pthread_cond_wait(_cond, cond._mu) != 0) {
    return false;
  }
  return true;
}

bool MCond::wait(MMutex& mutex, const unsigned int sec,
                 const unsigned int msec) throw()
{
  struct timeval now;
  struct timespec timeout;

  gettimeofday(&now, NULL);
  timeout.tv_sec = now.tv_sec + sec;
  timeout.tv_nsec = now.tv_usec * 1000 + msec;
  if (pthread_cond_timedwait(_cond, mutex._mu, &timeout) != 0) {
    return false;
  }
  return true;
}

const MCond& MCond::operator=(const MCond& cond) throw()
{
  _cond = cond._cond;
  return *this;
}
