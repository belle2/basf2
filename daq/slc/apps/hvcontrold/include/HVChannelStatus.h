#ifndef Belle2_HVChannelStatus_h
#define Belle2_HVChannelStatus_h

#include "daq/slc/base/DataObject.h"

namespace Belle2 {

  class HVChannelStatus : public DataObject {

  public:
    HVChannelStatus(unsigned int crate = 0, unsigned int slot = 0, unsigned int ch = 0);
    virtual ~HVChannelStatus() throw();

  public:
    std::string print_names();
    std::string print_values();

  public:
    unsigned int getCrate() const { return getUInt("crate"); }
    unsigned int getSlot() const { return getUInt("slot"); }
    unsigned int getChannel() const { return getUInt("channel"); }
    unsigned int getStatus() const { return getUInt("status"); }
    unsigned int getVoltageMonitored() const { return getUInt("voltage_monitored"); }
    unsigned int getCurrentMonitored() const { return getUInt("current_monitored"); }

    void setCrate(unsigned int v) { setUInt("crate", v); }
    void setSlot(unsigned int v) { setUInt("slot", v); }
    void setChannel(unsigned int v) { setUInt("channel", v); }
    void setStatus(unsigned int v) { setUInt("status", v); }
    void setVoltageMonitored(unsigned int v) { setUInt("voltage_monitored", v); }
    void setCurrentMonitored(unsigned int v) { setUInt("current_monitored", v); }

  };

};

#endif
