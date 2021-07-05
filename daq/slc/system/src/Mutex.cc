/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Mutex.h"

using namespace Belle2;

Mutex::Mutex()
{
  pthread_mutexattr_init(&m_attr);
  pthread_mutex_init(&m_mu, &m_attr);
}

Mutex::~Mutex()
{
}

bool Mutex::lock()
{
  if (pthread_mutex_lock(&m_mu) != 0) {
    return false;
  }
  return true;
}

bool Mutex::unlock()
{
  if (pthread_mutex_unlock(&m_mu) != 0) {
    return false;
  }
  return true;
}
