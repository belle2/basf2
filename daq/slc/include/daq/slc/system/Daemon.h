#ifndef _Daemon_hh
#define _Daemon_hh

namespace Belle2 {

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
