#ifndef Belle2_HVChannelInfo_h
#define Belle2_HVChannelInfo_h

#include "daq/slc/apps/hvcontrold/hv_channel_info.h"
#include "daq/slc/apps/hvcontrold/HVChannelConfig.h"
#include "daq/slc/apps/hvcontrold/HVChannelStatus.h"

#include <daq/slc/database/DBInterface.h>

namespace Belle2 {

  class HVChannelInfo {

  public:
    HVChannelInfo(uint32 master = 0, uint32 crate = 0,
                  uint32 slot = 0, uint32 channel = 0)
      : _config(master, crate, slot, channel) {}
    HVChannelInfo(const HVChannelInfo& info) {
      setConfig(info._config);
      setStatus(info._status);
    }
    virtual ~HVChannelInfo() throw() {}

  public:
    HVChannelConfig& getConfig() { return _config; }
    HVChannelStatus& getStatus() { return _status; }
    void setConfig(const HVChannelConfig& config);
    void setStatus(const HVChannelStatus& status);

  private:
    HVChannelConfig _config;
    HVChannelStatus _status;

  };

}

#endif
