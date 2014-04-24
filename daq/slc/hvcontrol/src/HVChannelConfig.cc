#include "daq/slc/hvcontrol/HVChannelConfig.h"

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

float HVChannelConfig::getVoltageDemand(HVDemand i) const
{
  std::string name = StringUtil::form("voltage_demand_%d", (int)i);
  return m_obj.getFloat(name);
}

float HVChannelConfig::getReserved(int i) const
{
  std::string name = StringUtil::form("reserved_%d", i);
  return m_obj.getFloat(name);
}

void HVChannelConfig::setVoltageDemand(HVDemand i, float v)
{
  std::string name = StringUtil::form("voltage_demand_%d", (int)i);
  m_obj.setFloat(name, v);
}

void HVChannelConfig::setReserved(int i, float v)
{
  std::string name = StringUtil::form("reserved_%d", i);
  m_obj.setFloat(name, v);
}
