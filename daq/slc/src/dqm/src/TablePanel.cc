#include "TablePanel.h"

#include <sstream>

using namespace Belle2;

const std::string TablePanel::ELEMENT_TAG = "table";

TablePanel::TablePanel() throw()
{

}

TablePanel::TablePanel(const std::string& name, int cols, int rows) throw()
  : Panel(name), _cols(cols), _rows(rows) {}

TablePanel::~TablePanel() throw() {}

std::string TablePanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' "
     << "col='" << _cols << "' "
     << "row='" << _rows << "' ";
  if (_width_ratio.size() > 0) ss << "width-ratio='" << _width_ratio << "' ";
  if (_height_ratio.size() > 0) ss << "height-ratio='" << _height_ratio << "' ";
  ss << ">\n";
  for (size_t n = 0; n < _sub_panel_v.size(); n++) {
    ss << _sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
