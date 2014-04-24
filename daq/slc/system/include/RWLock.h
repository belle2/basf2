#ifndef _Belle2_RWLock_hh
#define _Belle2_RWLock_hh

#include <pthread.h>

namespace Belle2 {

  class RWLock {

  public:
    static size_t size() throw() {
      return sizeof(pthread_rwlock_t);
    }

    // constructors and destructor
  public:
    RWLock() throw();
    ~RWLock() throw();

    // member functions;
  public:
    bool init() throw();
    bool rdlock() throw();
    bool wrlock() throw();
    bool unlock() throw();
    bool destroy() throw();

    // data members;
  private:
    pthread_rwlock_t _lock;

  };

};

#endif
