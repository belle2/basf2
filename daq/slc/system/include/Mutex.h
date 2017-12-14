#ifndef _Belle2_Mutex_hh
#define _Belle2_Mutex_hh

#include <pthread.h>

namespace Belle2 {

  class Mutex {

    friend class Cond;

    // constructors & destructors
  public:
    Mutex() throw();
    ~Mutex() throw();

    // member methods
  public:
    bool lock() throw();
    bool unlock() throw();

    // member data
  private:
    pthread_mutex_t m_mu;
    pthread_mutexattr_t m_attr;

  };

}

#endif
