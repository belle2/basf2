#ifndef _Belle2_HVApplyMessage_h
#define _Belle2_HVApplyMessage_h

#include <string>
#include <cstdlib>

namespace Belle2 {

  class HVApplyMessage {

  public:
    HVApplyMessage(int crate = -1, int slot = -1,
                   int channel = -1,
                   const std::string& param = "turnon",
                   const std::string& value = "false"):
      m_crate(crate), m_slot(slot),
      m_channel(channel), m_param(param),
      m_value(value) {}

  public:
    bool isAllCrates() const { return m_crate < 0; }
    bool isAllSlots() const { return m_slot < 0; }
    bool isAllChannels() const { return m_channel < 0; }
    bool isTurnon() const { return m_param == "turnon"; }
    bool isRampupSpeed() const { return m_param == "rampup_speed"; }
    bool isRampdownSpeed() const { return m_param == "rampdown_speed"; }
    bool isVoltageLimit() const { return m_param == "voltage_limit"; }
    bool isCurrentLimit() const { return m_param == "current_limit"; }
    bool isVoltageDemand() const { return m_param == "voltage_demand"; }
    bool getTurnon() const { return m_value == "true"; }
    float getRampupSpeed() const { return atof(m_value.c_str()); }
    float getRampdownSpeed() const { return atof(m_value.c_str()); }
    float getVoltageLimit() const { return atof(m_value.c_str()); }
    float getCurrentLimit() const { return atof(m_value.c_str()); }
    float getVoltageDemand() const { return atof(m_value.c_str()); }
    int getCrate() const { return m_crate; }
    int getSlot() const { return m_slot; }
    int getChannel() const { return m_channel; }
    const std::string& getParam() const { return m_param; }
    const std::string& getValue() const { return m_value; }

  public:
    void setParam(const std::string& param) { m_param = param; }
    void setValue(const std::string& value) { m_value = value; }

  private:
    int m_crate, m_slot, m_channel;
    std::string m_param;
    std::string m_value;
  };

}

#endif
