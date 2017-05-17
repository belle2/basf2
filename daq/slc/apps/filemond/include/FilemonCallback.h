#ifndef _Belle2_FilemonCallback_h
#define _Belle2_FilemonCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class FilemonCallback : public NSMCallback {

  public:
    FilemonCallback(const std::string& nodename, int timout = 5);
    virtual ~FilemonCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
