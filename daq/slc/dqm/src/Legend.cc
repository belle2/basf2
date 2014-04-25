#include "daq/slc/dqm/Legend.h"

#include <sstream>

using namespace Belle2;

const std::string Legend::ELEMENT_TAG = "legend";

void Legend::add(Histo* obj) throw()
{
  if (obj != NULL) {
    m_object_v.push_back(obj);
  }
}

std::string Legend::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (m_name.size() > 0) ss << "name='" << m_name << "' ";
  if (m_x >= 0 && m_y >= 0 && m_width >= 0 && m_height >= 0) {
    ss << "x='" << m_x << "' y='" << m_y << "' "
       << "width='" << m_width << "' height='" << m_height << "' ";
  }
  if (m_object_v.size() > 0) {
    ss << "list='";
    for (size_t n = 0;;) {
      ss << m_object_v[n]->getName();
      n++;
      if (n >= m_object_v.size()) break;
      ss << ",";
    }
    ss << "' ";
  }
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
