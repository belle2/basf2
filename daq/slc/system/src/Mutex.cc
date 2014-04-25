#include "daq/slc/system/Mutex.h"

using namespace Belle2;

Mutex::Mutex() throw()
{
  pthread_mutexattr_init(&m_attr);
  pthread_mutex_init(&m_mu, &m_attr);
}

Mutex::~Mutex() throw()
{
}

bool Mutex::lock() throw()
{
  if (pthread_mutex_lock(&m_mu) != 0) {
    return false;
  }
  return true;
}

bool Mutex::unlock() throw()
{
  if (pthread_mutex_unlock(&m_mu) != 0) {
    return false;
  }
  return true;
}
