#include <daq/slc/hvcontrol/HVValue.h>

#include <cstring>

using namespace Belle2;

HVValue::HVValue()
{
  m_turnon = false;
  m_rampup_speed = 0;
  m_rampdown_speed = 0;
  m_voltage_demand = 0;
  m_voltage_limit = 0;
  m_current_limit = 0;
}

HVValue::HVValue(const HVValue& c)
{
  m_turnon = c.m_turnon;
  m_rampup_speed = c.m_rampup_speed;
  m_rampdown_speed = c.m_rampdown_speed;
  m_voltage_demand = c.m_voltage_demand;
  m_voltage_limit = c.m_voltage_limit;
  m_current_limit = c.m_current_limit;
}

