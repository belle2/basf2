#include "daq/slc/dqm/EnvMonitorPackage.h"

using namespace Belle2;

EnvMonitorPackage::~EnvMonitorPackage() throw()
{
}

NSMData* EnvMonitorPackage::getData()
{
  return m_data;
}

void EnvMonitorPackage::setData(NSMData* data)
{
  m_data = data;
}

