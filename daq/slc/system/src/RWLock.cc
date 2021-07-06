/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/RWLock.h"

using namespace Belle2;

RWLock::RWLock() : m_lock() {}

RWLock::~RWLock() {}

bool RWLock::init()
{
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_rwlock_init(&m_lock, &attr);
  pthread_rwlockattr_destroy(&attr);
  return true;
}

bool RWLock::rdlock()
{
  if (pthread_rwlock_rdlock(&m_lock) != 0) {
    return false;
  } else {
    return true;
  }
}

bool RWLock::wrlock()
{
  if (pthread_rwlock_wrlock(&m_lock) != 0) {
    return false;
  }
  return true;
}

bool RWLock::unlock()
{
  if (pthread_rwlock_unlock(&m_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

bool RWLock::destroy()
{
  if (pthread_rwlock_destroy(&m_lock) != 0) {
    return true;
  } else {
    return false;
  }
}

