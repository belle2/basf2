#ifndef _Belle2_RCClientCallback_h
#define _Belle2_RCClientCallback_h

#include <daq/slc/nsm/RCCallback.h>

namespace Belle2 {

  class RCMaster;

  class RCClientCallback : public RCCallback {

  public:
    RCClientCallback(NSMNode* node, RCMaster* master);
    virtual ~RCClientCallback() throw();

  public:
    virtual void init() throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual void selfCheck() throw(NSMHandlerException);

  private:
    RCMaster* _master;
    bool _requested_once;

  };

}

#endif
