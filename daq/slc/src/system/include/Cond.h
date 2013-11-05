#ifndef _Belle2_Cond_hh
#define _Belle2_Cond_hh

#include "system/Mutex.h"

namespace Belle2 {

  class Cond {

  public:
    Cond();
    Cond(const Cond& cond);
    ~Cond() throw() {};

  public:
    bool init() throw();
    bool signal() throw();
    bool broadcast() throw();
    bool wait(Mutex& mutex) throw();
    bool wait(Mutex& mutex, const unsigned int sec,
              const unsigned int msec = 0) throw();
    bool destroy() throw();

  private:
    pthread_cond_t _cond_t;

  };
};
#endif
