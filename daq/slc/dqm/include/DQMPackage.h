#ifndef _Belle2_DQMPackage_h
#define _Belle2_DQMPackage_h

#include "daq/slc/dqm/MonitorPackage.h"
#include "daq/slc/dqm/DQMHistMap.h"

#include <string>
#include <map>

namespace Belle2 {

  class DQMPackage : public MonitorPackage {

  public:
    DQMPackage(const std::string& name) : MonitorPackage(name) {}
    virtual ~DQMPackage() throw() {}

  public:
    void setData(DQMHistMap* data);
    DQMHistMap* getData() { return _data; }

  private:
    DQMHistMap* _data;

  };

}

#endif


