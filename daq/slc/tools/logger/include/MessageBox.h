#ifndef _Belle2_MessageBox_hh
#define _Belle2_MessageBox_hh

#include "system/Mutex.h"
#include "system/Cond.h"

#include "base/LogMessage.h"

#include <queue>

namespace Belle2 {

  class MessageBox {

  public:
    MessageBox() {}
    ~MessageBox() {}

  public:
    LogMessage pop() throw();
    void push(const LogMessage& message) throw();

  private:
    Mutex _mutex;
    Cond _cond;
    std::queue<LogMessage> _message_v;

  };

};

#endif
