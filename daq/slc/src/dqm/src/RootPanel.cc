#include "RootPanel.hh"

#include <sstream>

using namespace B2DQM;

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
     << "title='" << _name << "' >\n";
  for (size_t n = 0; n < _sub_panel_v.size(); n++) {
    ss << _sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
