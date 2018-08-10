#ifndef _Belle2_ERecoRunControlCallback_hh
#define _Belle2_ERecoRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class ERecoMasterCallback;

  class ERecoRunControlCallback : public RCCallback {

  public:
    ERecoRunControlCallback(ERecoMasterCallback* callback);
    virtual ~ERecoRunControlCallback() {}

  public:
    virtual void initialize(const DBObject& obj);
    virtual bool perform(NSMCommunicator& com);

  private:
    ERecoMasterCallback* m_callback;

  };

}

#endif
