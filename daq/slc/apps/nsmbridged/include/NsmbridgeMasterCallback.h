#ifndef _Belle2_NsmbridgeMasterCallback_h
#define _Belle2_NsmbridgeMasterCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class NsmbridgeCallback;

  class NsmbridgeMasterCallback : public NSMCallback {

  public:
    NsmbridgeMasterCallback(NsmbridgeCallback* callback);
    virtual ~NsmbridgeMasterCallback() throw() {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    NsmbridgeCallback* m_callback;

  };

}

#endif

