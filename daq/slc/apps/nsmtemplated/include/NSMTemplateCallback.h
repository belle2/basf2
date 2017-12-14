#ifndef _Belle2_NSMTemplateCallback_h
#define _Belle2_NSMTemplateCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class NSMTemplateCallback : public NSMCallback {

  public:
    NSMTemplateCallback(const std::string& nodename, int timout = 5);
    virtual ~NSMTemplateCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
