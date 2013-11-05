#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "nsm/NSMCallback.h"

#include "base/Command.h"
#include "base/NSMNode.h"

namespace Belle2 {

  class RCCallback : public NSMCallback {

  public:
    RCCallback(NSMNode* node) throw();
    virtual ~RCCallback() throw() {}

  public:
    NSMNode* getNode() throw() { return _node; }

  public:
    virtual bool boot()   throw() { return true; }
    virtual bool load()   throw() { return true; }
    virtual bool start()  throw() { return true; }
    virtual bool stop()   throw() { return true; }
    virtual bool recover() throw() { return true; }
    virtual bool resume() throw() { return true; }
    virtual bool pause()  throw() { return true; }
    virtual bool abort()  throw() { return true; }
    virtual bool trigft()  throw() { return true; }
    virtual bool setparams()  throw();

  protected:
    virtual bool perform(NSMMessage& msg)
    throw(NSMHandlerException);

  private:
    NSMNode* _node;

  };

};

#endif
