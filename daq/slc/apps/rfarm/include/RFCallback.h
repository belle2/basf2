#ifndef _Belle2_RFCallback_h
#define _Belle2_RFCallback_h

#include "daq/rfarm/manager/RFConf.h"

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/base/StringUtil.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RFCallback : public NSMCallback {

  public:
    RFCallback(int timeout = 4) throw();
    virtual ~RFCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void configure() throw();
    virtual void unconfigure() throw();
    virtual void start() throw();
    virtual void restart() throw();
    virtual void stop() throw();
    virtual void resume() throw();
    virtual void pause() throw();
    virtual void status() throw();
    virtual bool perform(NSMCommunicator& com) throw();

  };

}

#endif
