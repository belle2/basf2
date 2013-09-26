#ifndef _B2DAQ_ProcessListener_hh
#define _B2DAQ_ProcessListener_hh

#include <system/Fork.hh>

namespace B2DAQ {

  class COPPERCallback;

  class ProcessListener {

  public:
    ProcessListener(COPPERCallback* callback, Fork forkfd) 
      : _callback(callback), _forkfd(forkfd) {}
    ~ProcessListener() {}

  public:
    void run();

  private:
    COPPERCallback* _callback;
    Fork _forkfd;

  };

}

#endif

