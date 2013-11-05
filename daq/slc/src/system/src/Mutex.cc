#include "system/Mutex.h"

using namespace Belle2;

Mutex::Mutex() throw()
{
  pthread_mutexattr_init(&_attr);
  pthread_mutex_init(&_mu, &_attr);
}

Mutex::~Mutex() throw()
{
}

bool Mutex::lock() throw()
{
  if (pthread_mutex_lock(&_mu) != 0) {
    return false;
  }
  return true;
}

bool Mutex::unlock() throw()
{
  if (pthread_mutex_unlock(&_mu) != 0) {
    return false;
  }
  return true;
}
