#ifndef _Belle2_HVMasterMasterCallback_h
#define _Belle2_HVMasterMasterCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class HVMasterCallback;

  class HVMasterMasterCallback : public RCCallback {

  public:
    HVMasterMasterCallback(HVMasterCallback* callback);
    virtual ~HVMasterMasterCallback() throw() {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    HVMasterCallback* m_callback;

  };

}

#endif

