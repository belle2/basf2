#include "LineProperty.h"

#include <sstream>

using namespace Belle2;

LineProperty::LineProperty(int width) throw()
  : Property(), _width(width) {}

LineProperty::LineProperty(const MonColor& color,
                           int width) throw()
  : Property(color),  _width(width) {}

std::string LineProperty::toString() const throw()
{
  std::stringstream ss;
  if (_use_color) ss << "line-color='" << _color.toString() << "' ";
  if (_width >= 0) ss << "line-width='" << _width << "' ";
  return ss.str();
}
