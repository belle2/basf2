#ifndef _Belle2_MCond_h
#define _Belle2_MCond_h

#include "daq/slc/readout/modules/MMutex.h"

#include <pthread.h>

namespace Belle2 {

  class MCond {

  public:
    static size_t size() throw() { return sizeof(pthread_cond_t); }

    // constructors and destructor
  public:
    MCond() throw();
    MCond(const MCond&) throw();
    MCond(void*) throw();
    ~MCond() throw();

    // member functions;
  public:
    bool init(void*) throw();
    bool init() throw();
    bool set(void*) throw();
    bool signal() throw();
    bool broadcast() throw();
    bool wait(MMutex& mutex) throw();
    bool wait(MMutex& mutex, const unsigned int sec,
              const unsigned int msec = 0) throw();
    bool destroy() throw();

  public:
    const MCond& operator=(const MCond&) throw();

    // data members;
  private:
    pthread_cond_t* _cond;

  };

};

#endif
