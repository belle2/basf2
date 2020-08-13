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
