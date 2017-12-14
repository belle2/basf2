#ifndef _Belle2_LogreaderCallback_h
#define _Belle2_LogreaderCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class LogreaderCallback : public NSMCallback {

  public:
    LogreaderCallback(const std::string& nodename, int timout = 5);
    virtual ~LogreaderCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void logset(const DAQLogMessage& msg) throw();

  };

}

#endif
