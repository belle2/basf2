#ifndef _Belle2_LogtestCallback_h
#define _Belle2_LogtestCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class LogtestCallback : public NSMCallback {

  public:
    LogtestCallback(const std::string& nodename, int timout = 5);
    virtual ~LogtestCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
