#ifndef _B2DAQ_Cond_hh
#define _B2DAQ_Cond_hh

#include "Mutex.hh"

namespace B2DAQ {

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
