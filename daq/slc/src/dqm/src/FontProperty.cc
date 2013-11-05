#include "dqm/FontProperty.h"

#include <sstream>

using namespace Belle2;

FontProperty::FontProperty(float size,
                           const std::string& weight,
                           const std::string& family) throw()
  : Property(), _size(size), _weight(weight), _family(family) {}

FontProperty::FontProperty(const MonColor& color, float size,
                           const std::string& weight,
                           const std::string& family) throw()
  : Property(color), _size(size), _weight(weight), _family(family) {}

std::string FontProperty::toString() const throw()
{
  std::stringstream ss;
  if (_use_color) ss << _label << "font-color='" << _color.toString() << "' ";
  if (_size > 0) ss << _label << "font-size='" << _size << "' ";
  if (_weight.size() > 0) ss << _label << "font-weight='" << _weight << "' ";
  if (_family.size() > 0) ss << _label << "font-family='" << _family << "' ";
  if (_align.size() > 0) ss << _label << "align='" << _align << "' ";
  if (_angle.size() > 0) ss << _label << "font-angle='" << _angle << "' ";
  return ss.str();
}
