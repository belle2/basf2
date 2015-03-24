#ifndef _Belle2_RFRunControlCallback_hh
#define _Belle2_RFRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class RFMasterCallback;

  class RFRunControlCallback : public RCCallback {

  public:
    RFRunControlCallback(RFMasterCallback* callback);
    virtual ~RFRunControlCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual bool perform(NSMCommunicator& com) throw();

  private:
    RFMasterCallback* m_callback;

  };

}

#endif
