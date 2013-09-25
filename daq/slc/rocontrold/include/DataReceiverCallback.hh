#ifndef _B2DAQ_DataReceiverCallback_hh
#define _B2DAQ_DataReceiverCallback_hh

#include <system/Fork.hh>

#include <node/DataReceiverNode.hh>

#include <nsm/DataReceiverData.hh>
#include <nsm/RunStatus.hh>

#include <runcontrol/RCCallback.hh>

namespace B2DAQ {

  class DataReceiverCallback : public RCCallback {

  private:
    int* openBuffer(size_t count, const char* path) throw();

  public:
    DataReceiverCallback(DataReceiverNode* node = NULL, 
			 DataReceiverData* _data = NULL);
    virtual ~DataReceiverCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool reboot() throw();
    virtual bool load() throw();
    virtual bool reload() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    DataReceiverNode* _node;
    DataReceiverData* _data;
    int* _buf_config;
    int* _buf_status;
    RunStatus* _status;
    Fork _fork_v[2];//0:eb0, 1:RecvStream1.py
    
  };

}

#endif
