#ifndef _B2DAQ_ROCallback_hh
#define _B2DAQ_ROCallback_hh

#include <system/Fork.hh>

#include <node/RONode.hh>

#include <runcontrol/RCCallback.hh>

namespace B2DAQ {

  class ROCallback : public RCCallback {

  private:
    int* openBuffer(size_t count, const char* path) throw();

  public:
    ROCallback(RONode* node = NULL);
    virtual ~ROCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool abort() throw();

  private:
    RONode* _node;
    int* _buf_config;
    int* _buf_status;
    Fork _fork_v[2];//0:eb0, 1:RecvStream1.py
    
  };

}

#endif
