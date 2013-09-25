#ifndef _B2DAQ_MessageBox_hh
#define _B2DAQ_MessageBox_hh

#include "RunControlMessage.hh"

#include <system/Mutex.hh>
#include <system/Cond.hh>

#include <queue>

namespace B2DAQ {

  class MessageBox {

  public:
    static MessageBox& get() { return __box; }

  private:
    static MessageBox __box;

  private:
    MessageBox(){}
    ~MessageBox(){}

  public:
    RunControlMessage pop() throw();
    void push(const RunControlMessage& message) throw();

  private:
    B2DAQ::Mutex _mutex;
    B2DAQ::Cond _cond;
    std::queue<RunControlMessage> _message_v;

  };

};

#endif
