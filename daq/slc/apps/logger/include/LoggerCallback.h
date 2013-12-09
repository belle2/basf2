#ifndef _Belle2_LoggerCallback_hh
#define _Belle2_LoggerCallback_hh

#include "daq/slc/nsm/LogCallback.h"

#include "daq/slc/base/NSMNode.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

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
