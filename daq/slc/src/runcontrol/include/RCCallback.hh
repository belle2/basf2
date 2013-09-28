#ifndef _B2DAQ_RCCallback_hh
#define _B2DAQ_RCCallback_hh

#include "RCCommand.hh"

#include <nsm/NSMCallback.hh>

#include <node/NSMNode.hh>

namespace B2DAQ {

  class RCCallback : public NSMCallback {

  public:
    RCCallback(NSMNode* node) throw();
    virtual ~RCCallback() throw() {}

  public:
    NSMNode* getNode() throw() { return _node; }

  public:
    virtual bool boot() throw() { return true; }
    virtual bool reboot() throw() { return true; }
    virtual bool load() throw() { return true; }
    virtual bool reload() throw() { return true; }
    virtual bool start() throw() { return true; }
    virtual bool stop() throw() { return true; }
    virtual bool resume() throw() { return true; }
    virtual bool pause() throw() { return true; }
    virtual bool recover() throw() { return true; }
    virtual bool abort() throw() { return true; }
    virtual bool stateCheck() throw() { return true; }

  protected:
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);

  private:
    NSMNode* _node;

  };

};

#endif
