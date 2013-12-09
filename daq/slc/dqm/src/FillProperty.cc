#include "daq/slc/dqm/FillProperty.h"

#include <sstream>

using namespace Belle2;

FillProperty::FillProperty(double transparency) throw()
  : Property(), _transparency(transparency) {}

FillProperty::FillProperty(const MonColor& color,
                           double transparency) throw()
  : Property(color),  _transparency(transparency) {}

std::string FillProperty::toString() const throw()
{
  std::stringstream ss;
  if (_use_color) ss << "fill-color='" << _color.toString() << "' ";
  if (_transparency >= 0) ss << "fill-transparency='" << _transparency << "' ";
  return ss.str();
}
