#include "daq/slc/dqm/MonArrow.h"

#include <sstream>

using namespace Belle2;

const std::string MonArrow::ELEMENT_TAG = "mon-arrow";

std::string MonArrow::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << m_name << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  if (m_x >= 0) ss << "position-x='" << m_x << "' ";
  if (m_y >= 0) ss << "position-y='" << m_y << "' ";
  if (m_width >= 0) ss << "width='" << m_width << "' ";
  if (m_height >= 0) ss << "length='" << m_height << "' ";
  if (m_head_width >= 0) ss << "head-width='" << m_head_width << "' ";
  if (m_head_length >= 0) ss << "head-length='" << m_head_length << "' ";
  ss << "/>\n";
  return ss.str();
}

