/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Mutex_hh
#define _Belle2_Mutex_hh

#include <pthread.h>

namespace Belle2 {

  class Mutex {

    friend class Cond;

    // constructors & destructors
  public:
    Mutex();
    ~Mutex();

    // member methods
  public:
    bool lock();
    bool unlock();

    // member data
  private:
    pthread_mutex_t m_mu;
    pthread_mutexattr_t m_attr;

  };

}

#endif
