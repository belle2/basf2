#include "dqm/TabbedPanel.h"

#include <sstream>

using namespace Belle2;

const std::string TabbedPanel::ELEMENT_TAG = "tabbedPanel";

TabbedPanel::TabbedPanel() throw()
{

}

TabbedPanel::TabbedPanel(const std::string& name) throw() : Panel(name) {}

TabbedPanel::~TabbedPanel() throw() {}

void TabbedPanel::add(const std::string& title, Panel* panel) throw()
{
  if (panel != NULL) {
    _sub_title_v.push_back(title);
    _sub_panel_v.push_back(panel);
  }
}

void TabbedPanel::add(Panel* panel) throw()
{
  if (panel != NULL) add(panel->getName(), panel);
}

std::string TabbedPanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' >\n";
  for (size_t n = 0; n < _sub_panel_v.size(); n++) {
    ss << "<tab title='" << _sub_title_v[n] << "' >\n"
       << _sub_panel_v[n]->toXML()
       << "</tab>\n";
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
