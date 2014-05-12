#include "daq/slc/hvcontrol/HVChannelConfig.h"

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

float HVChannelConfig::getVoltageDemand(int i) const
{
  std::string name = StringUtil::form("voltage_demand_%d", i);
  return m_obj.getFloat(name);
}

float HVChannelConfig::getReserved(int i) const
{
  std::string name = StringUtil::form("reserved_%d", i);
  return m_obj.getFloat(name);
}

void HVChannelConfig::setVoltageDemand(int i, float v)
{
  std::string name = StringUtil::form("voltage_demand_%d", i);
  m_obj.setFloat(name, v);
}

void HVChannelConfig::setReserved(int i, float v)
{
  std::string name = StringUtil::form("reserved_%d", i);
  m_obj.setFloat(name, v);
}
