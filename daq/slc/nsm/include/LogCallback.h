#ifndef _Belle2_LogCallback_hh
#define _Belle2_LogCallback_hh

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class LogCallback : public NSMCallback {

  public:
    LogCallback(NSMNode* node) throw();
    virtual ~LogCallback() throw() {}

  public:
    virtual bool log()   throw() { return true; }

  public:
    virtual bool perform(NSMMessage& msg)
    throw(NSMHandlerException);

  };

};

#endif
