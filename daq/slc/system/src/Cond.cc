#include "daq/slc/system/Cond.h"

#include <pthread.h>
#include <sys/time.h>

using namespace Belle2;

Cond::Cond()
{
  init();
}

Cond::Cond(const Cond& cond)
{
  _cond_t = cond._cond_t;
}

bool Cond::init() throw()
{
  pthread_condattr_t mattr;
  pthread_condattr_init(&mattr);
  pthread_condattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
  if (pthread_cond_init(&_cond_t, &mattr) != 0) {
    return false;
  }
  pthread_condattr_destroy(&mattr);

  return true;
}

bool Cond::signal() throw()
{
  if (pthread_cond_signal(&_cond_t) == 0) return true;
  else return false;
}

bool Cond::broadcast() throw()
{
  if (pthread_cond_broadcast(&_cond_t) == 0) return true;
  else return false;
}

bool Cond::wait(Mutex& mutex) throw()
{
  if (pthread_cond_wait(&_cond_t, &mutex._mu) != 0) {
    return false;
  }
  return true;
}

bool Cond::wait(Mutex& mutex, const unsigned int sec, const unsigned int msec) throw()
{
  struct timeval now;
  struct timespec timeout;

  gettimeofday(&now, NULL);
  timeout.tv_sec = now.tv_sec + sec;
  timeout.tv_nsec = now.tv_usec * 1000 + msec;
  int stat = 0;
  if ((stat = pthread_cond_timedwait(&_cond_t, &mutex._mu, &timeout)) != 0) {
    return false;
  }
  return true;
}

bool Cond::destroy() throw()
{
  if (pthread_cond_destroy(&_cond_t) == 0) return true;
  else return false;
}
