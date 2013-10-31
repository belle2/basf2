#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

#include <system/Fork.h>
#include <system/Mutex.h>

namespace Belle2 {

  class COPPERCallback;

  class ProcessListener {

  public:
    ProcessListener(COPPERCallback* callback, Fork forkfd)
      : _callback(callback), _forkfd(forkfd) {}
    ~ProcessListener() {}

  public:
    void run();
    void setRunning(bool is_running);

  private:
    COPPERCallback* _callback;
    Fork _forkfd;
    Mutex _mutex;
    bool _is_running;

  };

}

#endif

