#include "daq/slc/dqm/LineProperty.h"

#include <sstream>

using namespace Belle2;

LineProperty::LineProperty(int width) throw()
  : Property(), m_width(width) {}

LineProperty::LineProperty(const MonColor& color,
                           int width) throw()
  : Property(color),  m_width(width) {}

std::string LineProperty::toString() const throw()
{
  std::stringstream ss;
  if (m_use_color) ss << "line-color='" << m_color.toString() << "' ";
  if (m_width >= 0) ss << "line-width='" << m_width << "' ";
  return ss.str();
}
