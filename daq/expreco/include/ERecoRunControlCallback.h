#ifndef _Belle2_ERecoRunControlCallback_hh
#define _Belle2_ERecoRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class ERecoMasterCallback;

  class ERecoRunControlCallback : public RCCallback {

  public:
    ERecoRunControlCallback(ERecoMasterCallback* callback);
    virtual ~ERecoRunControlCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual bool perform(NSMCommunicator& com) throw();

  private:
    ERecoMasterCallback* m_callback;

  };

}

#endif
