#ifndef Belle2_HVChannelInfo_h
#define Belle2_HVChannelInfo_h

#include "base/DataObject.h"

namespace Belle2 {

  class HVChannelInfo : public DataObject {

  public:
    HVChannelInfo(unsigned int crate = 0, unsigned int slot = 0, unsigned int ch = 0);
    virtual ~HVChannelInfo() throw();

  public:
    unsigned int getCrate() const { return getUInt("crate"); }
    unsigned int getSlot() const { return getUInt("slot"); }
    unsigned int getChannel() const { return getUInt("channel"); }
    bool isSwitchOn() const { return getBool("switch_on"); }
    unsigned int getRampUpSpeed() const { return getUInt("rampup_speed"); }
    unsigned int getRampDownSpeed() const { return getUInt("rampdown_speed"); }
    unsigned int getVoltageDemand() const { return getUInt("voltage_demand"); }
    unsigned int getVoltageLimit() const { return getUInt("voltage_limit"); }
    unsigned int getCurrentLimit() const { return getUInt("current_limit"); }
    unsigned int getVoltageMonitored() const { return getUInt("voltage_monitored"); }
    unsigned int getCurrentMonitored() const { return getUInt("current_monitored"); }

    void setCrate(unsigned int v) { setUInt("crate", v); }
    void setSlot(unsigned int v) { setUInt("slot", v); }
    void setChannel(unsigned int v) { setUInt("channel", v); }
    void setSwitchOn(bool v) { setBool("switch_on", v); }
    void setRampUpSpeed(unsigned int v) { setUInt("rampup_speed", v); }
    void setRampDownSpeed(unsigned int v) { setUInt("rampdown_speed", v); }
    void setVoltageDemand(unsigned int v) { setUInt("voltage_demand", v); }
    void setVoltageLimit(unsigned int v) { setUInt("voltage_limit", v); }
    void setCurrentLimit(unsigned int v) { setUInt("current_limit", v); }
    void setVoltageMonitored(unsigned int v) { setUInt("voltage_monitored", v); }
    void setCurrentMonitored(unsigned int v) { setUInt("current_monitored", v); }

  };

};

#endif
