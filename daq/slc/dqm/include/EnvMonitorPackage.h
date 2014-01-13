#ifndef _Belle2_EnvMonitorPackage_h
#define _Belle2_EnvMonitorPackage_h

#include "daq/slc/dqm/MonitorPackage.h"

#include "daq/slc/nsm/NSMData.h"

#include <map>

namespace Belle2 {

  class EnvMonitorPackage : public MonitorPackage {

    friend class EnvMonitorMaster;

  public:
    EnvMonitorPackage(const std::string& name)
      : MonitorPackage(name) {}
    virtual ~EnvMonitorPackage() throw();

  public:
    NSMData* getData();
    void setData(NSMData* data);

  private:
    NSMData* _data;

  };

}

#define REGISTER_ENV_PACKAGE(classname) \
  extern "C" void* create##classname(const char* name)\
  {\
    return new Belle2::classname(name);\
  }

#endif


