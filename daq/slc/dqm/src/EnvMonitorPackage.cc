#include "daq/slc/dqm/EnvMonitorPackage.h"

using namespace Belle2;

EnvMonitorPackage::~EnvMonitorPackage() throw()
{
}

NSMData* EnvMonitorPackage::getData()
{
  return _data;
}

void EnvMonitorPackage::setData(NSMData* data)
{
  _data = data;
}

