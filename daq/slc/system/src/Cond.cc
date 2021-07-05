/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
  m_cond_t = cond.m_cond_t;
}

bool Cond::init()
{
  pthread_condattr_t mattr;
  pthread_condattr_init(&mattr);
  pthread_condattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
  if (pthread_cond_init(&m_cond_t, &mattr) != 0) {
    return false;
  }
  pthread_condattr_destroy(&mattr);

  return true;
}

bool Cond::signal()
{
  if (pthread_cond_signal(&m_cond_t) == 0) return true;
  else return false;
}

bool Cond::broadcast()
{
  if (pthread_cond_broadcast(&m_cond_t) == 0) return true;
  else return false;
}

bool Cond::wait(Mutex& mutex)
{
  if (pthread_cond_wait(&m_cond_t, &mutex.m_mu) != 0) {
    return false;
  }
  return true;
}

bool Cond::wait(Mutex& mutex, const unsigned int sec, const unsigned int msec)
{
  struct timeval now;
  struct timespec timeout;

  gettimeofday(&now, NULL);
  timeout.tv_sec = now.tv_sec + sec;
  timeout.tv_nsec = now.tv_usec * 1000 + msec;
  int stat = 0;
  if ((stat = pthread_cond_timedwait(&m_cond_t, &mutex.m_mu, &timeout)) != 0) {
    return false;
  }
  return true;
}

bool Cond::destroy()
{
  if (pthread_cond_destroy(&m_cond_t) == 0) return true;
  else return false;
}
