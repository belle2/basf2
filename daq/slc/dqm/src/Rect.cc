#include "daq/slc/dqm/Rect.h"

#include <sstream>

using namespace Belle2;

const std::string Rect::ELEMENT_TAG = "rect";

void Rect::setBounds(float x, float y, float width, float height) throw()
{
  _x = x;
  _y = y;
  _width = width;
  _height = height;
}

std::string Rect::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (_name.size() > 0) ss << "name='" << _name << "' ";
  if (_x >= 0 && _y >= 0 && _width >= 0 && _height >= 0) {
    ss << "bounds='" << _x << "," << _y << ","
       << _width << "," << _height << "' ";
  }
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
