#include "MonShape.h"

#include <sstream>

using namespace Belle2;

const std::string MonShape::ELEMENT_TAG = "mon-shape";

MonShape::MonShape() :
  MonObject(""),
  _x(-1), _y(-1), _width(-1), _height(-1) {}

MonShape::MonShape(const std::string& name)
  : MonObject(name),
    _x(-1), _y(-1), _width(-1), _height(-1) {}

std::string MonShape::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  if (_x >= 0 && _y >= 0 && _width >= 0 && _height >= 0) {
    ss << "bounds='" << _x << "," << _y << ","
       << _width << "," << _height << "' ";
  }
  ss << "/>\n";
  return ss.str();
}

void MonShape::setBounds(float x, float y, float width, float height) throw()
{
  _x = x;
  _y = y;
  _width = width;
  _height = height;
}

void MonShape::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
}

void MonShape::writeContents(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  writer.writeShort((short)_fill_color.getRed());
  writer.writeShort((short)_fill_color.getGreen());
  writer.writeShort((short)_fill_color.getBlue());
  writer.writeShort((short)_line_color.getRed());
  writer.writeShort((short)_line_color.getGreen());
  writer.writeShort((short)_line_color.getBlue());
}

void MonShape::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
}

void MonShape::readContents(Belle2::Reader& reader) throw(Belle2::IOException)
{
  short red = reader.readShort();
  short green = reader.readShort();
  short blue = reader.readShort();
  _fill_color.setRGB(red, green, blue);
  red = reader.readShort();
  green = reader.readShort();
  blue = reader.readShort();
  _line_color.setRGB(red, green, blue);
}

