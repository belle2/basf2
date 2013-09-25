#ifndef _Daemon_hh
#define _Daemon_hh

namespace B2DAQ {

  class Daemon {

  public:
    Daemon();
    virtual ~Daemon() throw();

  public:
    int start() throw();
    virtual void run() throw() = 0;
  
  };  

};

#endif
