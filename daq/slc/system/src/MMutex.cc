/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/MMutex.h"

using namespace Belle2;

MMutex::MMutex() {}

MMutex::MMutex(const MMutex& mutex)
{
  *this = mutex;
}

MMutex::MMutex(void* addr)
{
  set((pthread_mutex_t*)addr);
}

MMutex::~MMutex() {}

bool MMutex::init(void* addr)
{
  set(addr);
  init();
  return true;
}

bool MMutex::init()
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(m_mu, &attr);
  pthread_mutexattr_destroy(&attr);
  return true;
}

bool MMutex::set(void* addr)
{
  m_mu = (pthread_mutex_t*)addr;
  return true;
}

bool MMutex::lock()
{
  if (pthread_mutex_lock(m_mu) != 0) {
    return false;
  } else {
    return true;
  }
}

bool MMutex::trylock()
{
  if (pthread_mutex_lock(m_mu) != 0) {
    return false;
  }
  return true;
}

bool MMutex::unlock()
{
  if (pthread_mutex_unlock(m_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

bool MMutex::destroy()
{
  if (pthread_mutex_destroy(m_mu) != 0) {
    return true;
  } else {
    return false;
  }
}

const MMutex& MMutex::operator=(const MMutex& mutex)
{
  m_mu = mutex.m_mu;
  return *this;
}
