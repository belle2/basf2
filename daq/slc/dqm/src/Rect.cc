#include "daq/slc/dqm/Rect.h"

#include <sstream>

using namespace Belle2;

const std::string Rect::ELEMENT_TAG = "rect";

void Rect::setBounds(float x, float y, float width, float height) throw()
{
  m_x = x;
  m_y = y;
  m_width = width;
  m_height = height;
}

std::string Rect::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (m_name.size() > 0) ss << "name='" << m_name << "' ";
  if (m_x >= 0 && m_y >= 0 && m_width >= 0 && m_height >= 0) {
    ss << "bounds='" << m_x << "," << m_y << ","
       << m_width << "," << m_height << "' ";
  }
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
