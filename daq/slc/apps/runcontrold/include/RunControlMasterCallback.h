#ifndef _Belle2_RunControlMasterCallback_h
#define _Belle2_RunControlMasterCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class RunControlCallback;

  class RunControlMasterCallback : public RCCallback {

  public:
    RunControlMasterCallback(RunControlCallback* callback);
    virtual ~RunControlMasterCallback() throw() {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    RunControlCallback* m_callback;

  };

}

#endif

