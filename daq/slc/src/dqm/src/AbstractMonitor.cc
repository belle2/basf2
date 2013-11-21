#include "dqm/AbstractMonitor.h"

using namespace Belle2;

AbstractMonitor::AbstractMonitor(const std::string& name)
{
  m_package = new HistoPackage(name);
  m_panel = new RootPanel(name);
}

AbstractMonitor::~AbstractMonitor() throw()
{
  delete m_package;
  delete m_panel;
}
