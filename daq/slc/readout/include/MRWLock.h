#ifndef _Belle2_MRWLock_h
#define _Belle2_MRWLock_h

#include <pthread.h>

namespace Belle2 {

  class MRWLock {

  public:
    static size_t size() throw() {
      return sizeof(pthread_rwlock_t);
    }

    // data members;
  private:
    pthread_rwlock_t* _lock;

    // constructors and destructor
  public:
    MRWLock() throw();
    MRWLock(void*) throw();
    MRWLock(const MRWLock&) throw();
    ~MRWLock() throw();

    // member functions;
  public:
    bool init(void*) throw();
    bool init() throw();
    bool set(void*) throw();
    bool rdlock() throw();
    bool wrlock() throw();
    bool unlock() throw();
    bool destroy() throw();

  public:
    const MRWLock& operator=(const MRWLock&) throw();

  };

};

#endif
