#ifndef _Belle2_MMutex_h
#define _Belle2_MMutex_h

#include <pthread.h>

namespace Belle2 {

  class MMutex {

    friend class MCond;

  public:
    static size_t size() throw() { return sizeof(pthread_mutex_t); }

    // constructors and destructor
  public:
    MMutex() throw();
    MMutex(void*) throw();
    MMutex(const MMutex&) throw();
    ~MMutex() throw();

    // member functions;
  public:
    bool init(void*) throw();
    bool init() throw();
    bool set(void*) throw();
    bool lock() throw();
    bool trylock() throw();
    bool unlock() throw();
    bool destroy() throw();

  public:
    const MMutex& operator=(const MMutex&) throw();

    // data members;
  private:
    pthread_mutex_t* _mu;

  };

};

#endif
