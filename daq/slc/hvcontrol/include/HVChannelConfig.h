#ifndef Belle2_HVChannelConfig_h
#define Belle2_HVChannelConfig_h

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  enum HVDemand {
    STANDBY = 0, STANDBY2, STANDBY3, PEAK
  };

  class HVChannelConfig {

  public:
    HVChannelConfig(ConfigObject& obj) : m_obj(obj) {}
    HVChannelConfig(const HVChannelConfig& config)
      : m_obj(config.m_obj) {}
    ~HVChannelConfig() throw() {}

  public:
    void print() throw() { m_obj.print(); }
    int getCrate() const { return m_obj.getInt("crate"); }
    int getSlot() const { return m_obj.getInt("slot"); }
    int getChannel() const { return m_obj.getInt("channel"); }
    bool  isTurnOn() const { return m_obj.getBool("turnon"); }
    float getRampUpSpeed() const { return m_obj.getFloat("rampup_speed"); }
    float getRampDownSpeed() const { return m_obj.getFloat("rampdown_speed"); }
    float getVoltageDemand(HVDemand i) const;
    float getVoltageLimit() const { return m_obj.getFloat("voltage_limit"); }
    float getCurrentLimit() const { return m_obj.getFloat("current_limit"); }
    float getReserved(int i) const;
    const ConfigObject& get() const throw() { return m_obj; }

    void setCrate(int id) { m_obj.setInt("crate", id); }
    void setSlot(int id) { m_obj.setInt("slot", id); }
    void setChannel(int id) { m_obj.setInt("channel", id); }
    void setTurnOn(bool turnon) { m_obj.setFloat("turnon", turnon); }
    void setRampUpSpeed(float rampup) { m_obj.setFloat("rampup_speed", rampup); }
    void setRampDownSpeed(float rampdown) { m_obj.setFloat("rampdown_speed", rampdown); }
    void setVoltageDemand(HVDemand i, float voltage);
    void setVoltageLimit(float voltage) { m_obj.setFloat("voltage_limit", voltage); }
    void setCurrentLimit(float current) { m_obj.setFloat("current_limit", current); }
    void setReserved(int i, float reserved);

  private:
    ConfigObject m_obj;

  };

  typedef std::vector<HVChannelConfig> HVChannelConfigList;

}

#endif
