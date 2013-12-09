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
     << "name='" << _name << "' >\n";
  for (size_t n = 0; n < _sub_panel_v.size(); n++) {
    ss << _sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
