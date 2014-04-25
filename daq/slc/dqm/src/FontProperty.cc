#include "daq/slc/dqm/FontProperty.h"

#include <sstream>

using namespace Belle2;

FontProperty::FontProperty(float size,
                           const std::string& weight,
                           const std::string& family) throw()
  : Property(), m_size(size), m_weight(weight), m_family(family) {}

FontProperty::FontProperty(const MonColor& color, float size,
                           const std::string& weight,
                           const std::string& family) throw()
  : Property(color), m_size(size), m_weight(weight), m_family(family) {}

std::string FontProperty::toString() const throw()
{
  std::stringstream ss;
  if (m_use_color) ss << m_label << "font-color='" << m_color.toString() << "' ";
  if (m_size > 0) ss << m_label << "font-size='" << m_size << "' ";
  if (m_weight.size() > 0) ss << m_label << "font-weight='" << m_weight << "' ";
  if (m_family.size() > 0) ss << m_label << "font-family='" << m_family << "' ";
  if (m_align.size() > 0) ss << m_label << "align='" << m_align << "' ";
  if (m_angle.size() > 0) ss << m_label << "font-angle='" << m_angle << "' ";
  return ss.str();
}
