#include "daq/slc/dqm/Pad.h"

#include <sstream>

using namespace Belle2;

const std::string Pad::ELEMENT_TAG = "pad";

std::string Pad::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (m_name.size() > 0) ss << "name='" << m_name << "' ";
  ss << "x='" << m_x << "' y='" << m_y << "' "
     << "width='" << m_width << "' height='" << m_height << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
