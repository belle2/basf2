/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RWLock_hh
#define _Belle2_RWLock_hh

#include <pthread.h>

namespace Belle2 {

  class RWLock {

  public:
    static size_t size()
    {
      return sizeof(pthread_rwlock_t);
    }

    // constructors and destructor
  public:
    RWLock();
    ~RWLock();

    // member functions;
  public:
    bool init();
    bool rdlock();
    bool wrlock();
    bool unlock();
    bool destroy();

    // data members;
  private:
    pthread_rwlock_t m_lock;

  };

};

#endif
