#ifndef _Belle2_EnvMonitorPackage_h
#define _Belle2_EnvMonitorPackage_h

#include "daq/slc/dqm/MonitorPackage.h"

#include "daq/slc/nsm/NSMData.h"

#include <map>

namespace Belle2 {

  typedef std::map<std::string, NSMData*> NSMDataMap;

  class EnvMonitorPackage : public MonitorPackage {

    friend class EnvMonitorMaster;

  public:
    EnvMonitorPackage(const std::string& name)
      : MonitorPackage(name) {}
    virtual ~EnvMonitorPackage() throw();

  public:
    NSMData* getData(const std::string& name);
    void addData(NSMData* data);
    bool hasData(const std::string& name);
    NSMDataMap& getDataMap() { return _data_m; }

  private:
    NSMDataMap _data_m;

  };

}

#endif


