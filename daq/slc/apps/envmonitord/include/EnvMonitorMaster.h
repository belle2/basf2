#ifndef _Belle2_EnvMonitorMaster_h
#define _Belle2_EnvMonitorMaster_h

#include "daq/slc/apps/MonitorMaster.h"

#include <daq/slc/dqm/EnvMonitorPackage.h>

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class EnvMonitorMaster : public MonitorMaster {

  public:
    EnvMonitorMaster(NSMCommunicator* comm)
      : MonitorMaster(), _comm(comm) {}
    ~EnvMonitorMaster() throw() {}

  public:
    void add(EnvMonitorPackage* monitor) {
      addManager(new PackageManager(monitor));
    }

  public:
    void run();

  private:
    NSMCommunicator* _comm;

  };

}

#endif
