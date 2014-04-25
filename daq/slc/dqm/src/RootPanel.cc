#include "daq/slc/dqm/RootPanel.h"

#include <sstream>

using namespace Belle2;

const std::string RootPanel::ELEMENT_TAG = "package";

RootPanel::RootPanel() throw()
{

}

RootPanel::RootPanel(const std::string& name) throw() : Panel(name) {}

RootPanel::~RootPanel() throw() {}

std::string RootPanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<?xml version='1.0'?>\n"
     << "<" << ELEMENT_TAG << " "
     << "title='" << m_name << "' >\n";
  for (size_t n = 0; n < m_sub_panel_v.size(); n++) {
    ss << m_sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
