#ifndef _Belle2_Rc2eb1BridgeCallback_h
#define _Belle2_Rc2eb1BridgeCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class Rc2eb1Callback;

  class Rc2eb1BridgeCallback : public RCCallback {

  public:
    Rc2eb1BridgeCallback(Rc2eb1Callback* callback, const std::string& name);
    virtual ~Rc2eb1BridgeCallback() throw() {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    Rc2eb1Callback* m_callback;

  };

}

#endif

