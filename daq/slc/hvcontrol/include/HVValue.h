#ifndef Belle2_HVValue_h
#define Belle2_HVValue_h

#include <vector>
#include <string>

namespace Belle2 {

  class HVValue {


  public:
    HVValue();
    HVValue(const HVValue& config) ;
    ~HVValue() throw() {}

  public:
    bool  isTurnOn() const { return m_turnon; }
    float getRampUpSpeed() const { return m_rampup_speed; }
    float getRampDownSpeed() const { return m_rampdown_speed; }
    float getVoltageDemand() const { return m_voltage_demand; }
    float getVoltageLimit() const { return m_voltage_limit; }
    float getCurrentLimit() const { return m_current_limit; }
    const std::string& getConfigName() const { return m_configname; }

    void setTurnOn(bool turnon) { m_turnon = turnon; }
    void setRampUpSpeed(float rampup) { m_rampup_speed = rampup; }
    void setRampDownSpeed(float rampdown) { m_rampdown_speed = rampdown; }
    void setVoltageDemand(float voltage) { m_voltage_demand = voltage; }
    void setVoltageLimit(float voltage) { m_voltage_limit = voltage; }
    void setCurrentLimit(float current) { m_current_limit = current; }
    void setConfigName(const std::string& name) { m_configname = name; }

  private:
    bool m_turnon;
    float m_rampup_speed;
    float m_rampdown_speed;
    float m_voltage_demand;
    float m_voltage_limit;
    float m_current_limit;
    std::string m_configname;

  };

  typedef std::vector<HVValue> HVValueSet;
  typedef std::vector<HVValueSet> HVValueSetList;

}

#endif
