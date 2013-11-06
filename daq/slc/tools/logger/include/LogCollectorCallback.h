#ifndef _Belle2_LogCollectorCallback_hh
#define _Belle2_LogCollectorCallback_hh

#include "nsm/LogCallback.h"

#include "base/NSMNode.h"

#include "system/Fork.h"
#include "system/PThread.h"

namespace Belle2 {

  class LogCollectorCallback : public LogCallback {

  public:
    LogCollectorCallback(NSMNode* node);
    virtual ~LogCollectorCallback() throw();

  public:
    virtual bool log() throw();

  };

}

#endif
