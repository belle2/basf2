#ifndef Belle2_HVValue_h
#define Belle2_HVValue_h

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class HVValue {

  public:
    HVValue(ConfigObject* obj) : m_obj(obj) {}
    HVValue(const HVValue& config)
      : m_obj(config.m_obj) {}
    ~HVValue() throw() {}

  public:
    void print() throw() { m_obj->print(); }
    float getRampUpSpeed() const { return m_obj->getFloat("rampup_speed"); }
    float getRampDownSpeed() const { return m_obj->getFloat("rampdown_speed"); }
    float getVoltageDemand() const { return m_obj->getFloat("voltage_demand"); }
    float getVoltageLimit() const { return m_obj->getFloat("voltage_limit"); }
    float getCurrentLimit() const { return m_obj->getFloat("current_limit"); }
    void set(ConfigObject* obj) throw() { m_obj = obj; }
    ConfigObject& get() throw() { return *m_obj; }

    void setRampUpSpeed(float rampup) { m_obj->setFloat("rampup_speed", rampup); }
    void setRampDownSpeed(float rampdown) { m_obj->setFloat("rampdown_speed", rampdown); }
    void setVoltageDemand(float voltage) { m_obj->setFloat("voltage_demand", voltage); }
    void setVoltageLimit(float voltage) { m_obj->setFloat("voltage_limit", voltage); }
    void setCurrentLimit(float current) { m_obj->setFloat("current_limit", current); }

  private:
    ConfigObject* m_obj;

  };

  typedef std::vector<HVValue> HVValueSet;
  typedef std::vector<HVValueSet> HVValueSetList;

}

#endif
