#ifndef _Belle2_LogCallback_hh
#define _Belle2_LogCallback_hh

#include "nsm/NSMCallback.h"

#include "base/Command.h"
#include "base/NSMNode.h"

namespace Belle2 {

  class LogCallback : public NSMCallback {

  public:
    LogCallback(NSMNode* node) throw();
    virtual ~LogCallback() throw() {}

  public:
    NSMNode* getNode() throw() { return _node; }

  public:
    virtual bool log()   throw() { return true; }

  protected:
    virtual bool perform(NSMMessage& msg)
    throw(NSMHandlerException);

  private:
    NSMNode* _node;

  };

};

#endif
