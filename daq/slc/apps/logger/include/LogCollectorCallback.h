#ifndef _Belle2_LogCollectorCallback_hh
#define _Belle2_LogCollectorCallback_hh

#include "daq/slc/nsm/LogCallback.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

namespace Belle2 {

  class LogDBManager;

  class LogCollectorCallback : public LogCallback {

  public:
    LogCollectorCallback(NSMNode* node, LogDBManager* man);
    virtual ~LogCollectorCallback() throw();

  public:
    virtual bool log() throw();

  private:
    LogDBManager* _man;

  };

}

#endif
