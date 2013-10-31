#ifndef _Belle2_MessageBox_hh
#define _Belle2_MessageBox_hh

#include "RunControlMessage.h"

#include <system/Mutex.h>
#include <system/Cond.h>

#include <queue>

namespace Belle2 {

  class MessageBox {

  public:
    static MessageBox& get() { return __box; }

  private:
    static MessageBox __box;

  private:
    MessageBox() {}
    ~MessageBox() {}

  public:
    RunControlMessage pop() throw();
    void push(const RunControlMessage& message) throw();

  private:
    Belle2::Mutex _mutex;
    Belle2::Cond _cond;
    std::queue<RunControlMessage> _message_v;

  };

};

#endif
