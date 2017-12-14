#ifndef _Belle2_TTDMasterCallback_h
#define _Belle2_TTDMasterCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class TTDACallback;

  class TTDMasterCallback : public RCCallback {

  public:
    TTDMasterCallback(TTDACallback* callback);
    virtual ~TTDMasterCallback() throw() {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    TTDACallback* m_callback;

  };

}

#endif

