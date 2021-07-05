/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Cond_hh
#define _Belle2_Cond_hh

#include "daq/slc/system/Mutex.h"

namespace Belle2 {

  class Cond {

  public:
    Cond();
    Cond(const Cond& cond);
    ~Cond() {};

  public:
    bool init();
    bool signal();
    bool broadcast();
    bool wait(Mutex& mutex);
    bool wait(Mutex& mutex, const unsigned int sec,
              const unsigned int msec = 0);
    bool destroy();

  private:
    pthread_cond_t m_cond_t;

  };
};
#endif
