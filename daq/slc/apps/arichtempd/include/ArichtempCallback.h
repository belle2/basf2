#ifndef _Belle2_ArichtempCallback_h
#define _Belle2_ArichtempCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class ArichtempCallback : public NSMCallback {

  public:
    ArichtempCallback(const std::string& nodename, int timout = 5);
    virtual ~ArichtempCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
