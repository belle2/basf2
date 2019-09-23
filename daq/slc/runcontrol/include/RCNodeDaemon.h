#ifndef _Belle2_RCNodeDaemon_h
#define _Belle2_RCNodeDaemon_h

#include "daq/slc/database/DBInterface.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  class RCNodeDaemon {

  public:
    RCNodeDaemon(ConfigFile& config,
                 RCCallback* callback,
                 RCCallback* callback2 = NULL,
                 DBInterface* db = NULL);
    ~RCNodeDaemon() {}

  public:
    void run();

  private:
    std::string m_title;
    NSMNodeDaemon m_daemon;

  };

}

#endif
