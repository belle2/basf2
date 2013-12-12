#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/Mutex.h"

#include <string>

namespace Belle2 {

  class RCCallback;

  class ProcessListener {

  public:
    ProcessListener(RCCallback* callback, Fork& forkfd,
                    const std::string& process_name)
      : _callback(callback), _forkfd(forkfd),
        _process_name(process_name) {}
    ~ProcessListener() {}

  public:
    void run();
    void setRunning(bool is_running);

  private:
    RCCallback* _callback;
    Fork& _forkfd;
    std::string _process_name;
    Mutex _mutex;
    bool _is_running;

  };

}

#endif

