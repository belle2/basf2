#ifndef _Belle2_LoggerCallback_hh
#define _Belle2_LoggerCallback_hh

#include "nsm/LogCallback.h"

#include "base/NSMNode.h"

#include "system/Fork.h"
#include "system/PThread.h"

namespace Belle2 {

  class LoggerCallback : public LogCallback {

  public:
    LoggerCallback(NSMNode* node, NSMCommunicator* comm);
    virtual ~LoggerCallback() throw();

  public:
    virtual bool log() throw();

  private:
    NSMCommunicator* _comm;

  };

}

#endif
