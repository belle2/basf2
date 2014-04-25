#include "daq/slc/dqm/TablePanel.h"

#include <sstream>

using namespace Belle2;

const std::string TablePanel::ELEMENT_TAG = "table";

TablePanel::TablePanel() throw()
{

}

TablePanel::TablePanel(const std::string& name, int cols, int rows) throw()
  : Panel(name), m_cols(cols), m_rows(rows) {}

TablePanel::~TablePanel() throw() {}

std::string TablePanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << m_name << "' "
     << "col='" << m_cols << "' "
     << "row='" << m_rows << "' ";
  if (m_width_ratio.size() > 0) ss << "width-ratio='" << m_width_ratio << "' ";
  if (m_height_ratio.size() > 0) ss << "height-ratio='" << m_height_ratio << "' ";
  ss << ">\n";
  for (size_t n = 0; n < m_sub_panel_v.size(); n++) {
    ss << m_sub_panel_v[n]->toXML();
  }
  ss << "</" << ELEMENT_TAG << ">\n";
  return ss.str();
}
