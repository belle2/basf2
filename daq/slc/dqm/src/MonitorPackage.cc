#include "daq/slc/dqm/MonitorPackage.h"

using namespace Belle2;

MonitorPackage::MonitorPackage(const std::string& name)
{
  m_package = new HistoPackage(name);
  m_panel = new RootPanel(name);
}

MonitorPackage::~MonitorPackage() throw()
{
  delete m_package;
  delete m_panel;
}
