#include "daq/slc/dqm/EnvMonitorPackage.h"

using namespace Belle2;

NSMData* EnvMonitorPackage::getData(const std::string& name)
{
  return (hasData(name)) ? _data_m[name] : NULL;
}

void EnvMonitorPackage::addData(NSMData* data)
{
  _data_m.insert(NSMDataMap::value_type(data->getName(), data));
}

bool EnvMonitorPackage::hasData(const std::string& name)
{
  return _data_m.find(name) != _data_m.begin();
}
