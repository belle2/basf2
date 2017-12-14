#ifndef _Belle2_HltscriptCallback_h
#define _Belle2_HltscriptCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class HltscriptCallback : public NSMCallback {

  public:
    HltscriptCallback(const std::string& nodename, int timout = 5);
    virtual ~HltscriptCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
