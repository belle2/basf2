#ifndef _Belle2_DQMViewMaster_h
#define _Belle2_DQMViewMaster_h

#include "daq/slc/apps/MonitorMaster.h"

#include <daq/slc/dqm/DQMPackage.h>
#include <daq/slc/dqm/DQMHistMap.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class DQMPackageUpdater;

  class DQMViewMaster : public MonitorMaster {

  public:
    void setDirectory(const std::string& dir) { _directory = dir; }
    void add(DQMPackage* monitor) {
      addManager(new PackageManager(monitor));
    }

  public:
    void run();

  private:
    std::string _directory;
    std::vector<DQMPackageUpdater*> _updater_v;

  };

}

#endif
