#include "daq/slc/dqm/ScrollPanel.h"

#include <sstream>

using namespace Belle2;

const std::string ScrollPanel::ELEMENT_TAG = "scrollPanel";

ScrollPanel::ScrollPanel() throw()
{

}

ScrollPanel::ScrollPanel(const std::string& name) throw() : Panel(name) {}

ScrollPanel::~ScrollPanel() throw() {}

std::string ScrollPanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << m_name << "' >\n";
  for (size_t n = 0; n < m_sub_panel_v.size(); n++) {
    ss << m_sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
