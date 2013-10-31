#include "Pad.h"

#include <sstream>

using namespace Belle2;

const std::string Pad::ELEMENT_TAG = "pad";

std::string Pad::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (_name.size() > 0) ss << "name='" << _name << "' ";
  ss << "x='" << _x << "' y='" << _y << "' "
     << "width='" << _width << "' height='" << _height << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
