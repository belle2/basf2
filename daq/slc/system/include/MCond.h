/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_MCond_h
#define _Belle2_MCond_h

#include "daq/slc/system/MMutex.h"

#include <pthread.h>

namespace Belle2 {

  class MCond {

  public:
    static size_t size() { return sizeof(pthread_cond_t); }

    // constructors and destructor
  public:
    MCond();
    MCond(const MCond&);
    MCond(void*);
    ~MCond();

    // member functions;
  public:
    bool init(void*);
    bool init();
    bool set(void*);
    bool signal();
    bool broadcast();
    bool wait(MMutex& mutex);
    bool wait(MMutex& mutex, const unsigned int sec,
              const unsigned int msec = 0);
    bool destroy();

  public:
    const MCond& operator=(const MCond&);

    // data members;
  private:
    pthread_cond_t* m_cond;

  };

};

#endif
