#include "daq/slc/dqm/FillProperty.h"

#include <sstream>

using namespace Belle2;

FillProperty::FillProperty(double transparency) throw()
  : Property(), m_transparency(transparency) {}

FillProperty::FillProperty(const MonColor& color,
                           double transparency) throw()
  : Property(color),  m_transparency(transparency) {}

std::string FillProperty::toString() const throw()
{
  std::stringstream ss;
  if (m_use_color) ss << "fill-color='" << m_color.toString() << "' ";
  if (m_transparency >= 0) ss << "fill-transparency='" << m_transparency << "' ";
  return ss.str();
}
