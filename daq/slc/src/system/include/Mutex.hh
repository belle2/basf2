#ifndef _B2DAQ_Mutex_hh
#define _B2DAQ_Mutex_hh

#include <pthread.h>

namespace B2DAQ {

  class Mutex {

    friend class Cond;

    // member data
  private:
    pthread_mutex_t _mu;
    pthread_mutexattr_t _attr;

    // constructors & destructors
  public:
    Mutex() throw();
    ~Mutex() throw();

    // member methods
  public:
    bool lock() throw();
    bool unlock() throw();

  };

}

#endif
