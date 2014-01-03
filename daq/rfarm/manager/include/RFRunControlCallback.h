#ifndef _Belle2_RFRunControlCallback_hh
#define _Belle2_RFRunControlCallback_hh

#include <daq/slc/nsm/RCCallback.h>

namespace Belle2 {

  class RFMaster;
  class RFMasterCallback;

  class RFRunControlCallback : public RCCallback {

  public:
    RFRunControlCallback(NSMNode* node, RFMaster* master,
                         RFMasterCallback* callback);
    virtual ~RFRunControlCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    RFMaster* _master;
    RFMasterCallback* _callback;

  };

}

#endif
