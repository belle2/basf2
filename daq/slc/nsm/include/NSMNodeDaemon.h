#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "daq/slc/nsm/NSMCallback.h"
#include "daq/slc/nsm/NSMData.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <queue>

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMCallback* callback,
                  const std::string host = "", int port = -1);
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();
    void init() throw(NSMHandlerException);
    NSMCommunicator* getCommunicator() { return m_com; }

  private:
    NSMCallback* m_callback;
    NSMCommunicator* m_com;

  };

}

#endif
