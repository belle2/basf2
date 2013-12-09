#ifndef _Belle2_LogMessageBox_hh
#define _Belle2_LogMessageBox_hh

#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"

#include "daq/slc/base/SystemLog.h"

#include <queue>

namespace Belle2 {

  class LogMessageBox {

  public:
    LogMessageBox() {}
    ~LogMessageBox() {}

  public:
    SystemLog pop() throw();
    void push(const SystemLog& message) throw();

  private:
    Mutex _mutex;
    Cond _cond;
    std::queue<SystemLog> _message_v;

  };

};

#endif
