/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_MMutex_h
#define _Belle2_MMutex_h

#include <pthread.h>

#ifdef __CINT__
#undef __GNUC__
#define _SYS__SELECT_H_
struct pthread_cond_t;
struct pthread_mutex_t;
#endif

namespace Belle2 {

  class MMutex {

    friend class MCond;

  public:
    static size_t size() { return sizeof(pthread_mutex_t); }

    // constructors and destructor
  public:
    MMutex();
    MMutex(void*);
    MMutex(const MMutex&);
    ~MMutex();

    // member functions;
  public:
    bool init(void*);
    bool init();
    bool set(void*);
    bool lock();
    bool trylock();
    bool unlock();
    bool destroy();

  public:
    const MMutex& operator=(const MMutex&);

    // data members;
  private:
    pthread_mutex_t* m_mu;

  };

};

#endif
