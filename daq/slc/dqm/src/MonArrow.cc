#include "daq/slc/dqm/MonArrow.h"

#include <sstream>

using namespace Belle2;

const std::string MonArrow::ELEMENT_TAG = "mon-arrow";

std::string MonArrow::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  if (_x >= 0) ss << "position-x='" << _x << "' ";
  if (_y >= 0) ss << "position-y='" << _y << "' ";
  if (_width >= 0) ss << "width='" << _width << "' ";
  if (_height >= 0) ss << "length='" << _height << "' ";
  if (_head_width >= 0) ss << "head-width='" << _head_width << "' ";
  if (_head_length >= 0) ss << "head-length='" << _head_length << "' ";
  ss << "/>\n";
  return ss.str();
}

